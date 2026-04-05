using System;

namespace Eno
{
    public class SpellbookController : ScriptBase
    {
        private static SpellbookController? _instance;

        private uint       _spellbookUIID  = 0;
        private uint       _spellbookNewID = 0;
        private uint       _fIconID        = 0;
        private AudioNode? _spellbookAudio = null;

        // ── Spell unlock fade-in ──────────────────────────────────────────────
        private string? _pendingKBTexture   = null;  // cleared after fade completes
        private string? _pendingCtrlTexture = null;
        private string? _activeKBTexture    = null;  // persists after fade; drives per-frame texture sync
        private string? _activeCtrlTexture  = null;
        private bool    _fadeActive         = false;
        private float   _fadeTimer          = 0f;
        private const float FadeDuration    = 2f;

        // Rooms check these to avoid overwriting the spellbook texture before or during the fade.
        public static bool FadeActive  => _instance?._fadeActive ?? false;
        public static bool FadePending => _instance?._pendingKBTexture != null;

        // Called by room scripts when a scroll is picked up.
        // Stores the new-spell textures so the fade triggers on next spellbook open.
        public static void QueueFadeIn(string kbTexture, string ctrlTexture)
        {
            if (_instance == null) return;
            _instance._pendingKBTexture   = kbTexture;
            _instance._pendingCtrlTexture = ctrlTexture;
            // _activeKBTexture / _activeCtrlTexture intentionally NOT updated here.
            // They still hold the previous spell's texture so the per-frame sync
            // keeps showing the old spellbook until the fade completes.
        }

        // ── Lifecycle ────────────────────────────────────────────────────────
        public void Begin()
        {
            _instance       = this;
            _spellbookUIID  = Bridge.GetNode("Spellbook");
            _spellbookNewID = Bridge.GetNode("SpellbookNew");
            _fIconID        = Bridge.GetNode("F_Icon");
            _spellbookAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            if (_spellbookAudio != null) _spellbookAudio.group = "BGM";

            // SpellbookNew starts hidden and fully transparent
            if (_spellbookNewID != 0)
            {
                Bridge.SetVisibility(_spellbookNewID, false);
                Bridge.SetColor(_spellbookNewID, 1f, 1f, 1f, 0f);
            }
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            if (_fIconID != 0)
                Bridge.SetTexturePath(_fIconID, isController ? "UI-SpellbookIcon-Controller" : "UI-Spellbook-Icon-F");

            if ((Input.isKeyPressed(KeyCode.F) || (isController && Input.isGamepadButtonPressed(GamepadButton.Y)))
                && _spellbookUIID != 0
                && !Scroll_Dialouge_Script.DialogueActive
                && !PauseManager.PauseMenuOpen
                && !PauseManager.CameraPanning)
            {
                bool nowOpen = !PauseManager.SpellbookOpen;
                Bridge.SetVisibility(_spellbookUIID, nowOpen);
                SetChildButtonsActive(_spellbookUIID, nowOpen);
                if (nowOpen)
                {
                    // Play sound BEFORE SetSpellbook — Sync() will pause audio immediately after
                    if (_spellbookAudio != null) { _spellbookAudio.sound = "Spellbook_Open_01"; _spellbookAudio.play(); }
                    Bridge.SetGroupPaused("SFX", true);
                    PauseManager.SetSpellbook(true);

                    // Start fade-in overlay if a new spell is queued
                    if (_pendingKBTexture != null && !_fadeActive)
                    {
                        // Pin the old texture onto the spellbook right as it opens so it
                        // shows correctly regardless of any canvas visibility reset.
                        if (_spellbookUIID != 0 && _activeKBTexture != null)
                            Bridge.SetTexturePath(_spellbookUIID, isController ? _activeCtrlTexture! : _activeKBTexture!);
                        StartFade(isController);
                    }
                }
                else
                {
                    // If closed mid-fade, snap to the completed state immediately
                    if (_fadeActive) CompleteFade();

                    PauseManager.SetSpellbook(false);
                    Bridge.SetGroupPaused("SFX", false);
                    if (_spellbookAudio != null) { _spellbookAudio.sound = "Spellbook_Close_01"; _spellbookAudio.play(); }
                }
            }

            // Advance the fade each frame
            if (_fadeActive)
            {
                _fadeTimer += Time.deltaTime;
                float alpha = Math.Min(_fadeTimer / FadeDuration, 1f);
                if (_spellbookNewID != 0)
                    Bridge.SetColor(_spellbookNewID, 1f, 1f, 1f, alpha);

                if (_fadeTimer >= FadeDuration)
                    CompleteFade();
            }

            // Keep the main spellbook texture in sync every frame (handles controller/keyboard
            // switching and guards against canvas visibility resets).
            // Blocked only while a fade is actively running — _activeKBTexture holds the OLD
            // texture until CompleteFade promotes pending → active, so this never leaks the
            // new texture prematurely.
            if (_spellbookUIID != 0 && _activeKBTexture != null && !_fadeActive)
            {
                Bridge.SetTexturePath(_spellbookUIID, isController ? _activeCtrlTexture! : _activeKBTexture!);
            }
        }

        // ── Fade helpers ─────────────────────────────────────────────────────
        private void StartFade(bool isController)
        {
            string tex = isController ? _pendingCtrlTexture! : _pendingKBTexture!;
            if (_spellbookNewID != 0)
            {
                Bridge.SetTexturePath(_spellbookNewID, tex);
                Bridge.SetColor(_spellbookNewID, 1f, 1f, 1f, 0f);
                Bridge.SetVisibility(_spellbookNewID, true);
            }
            _fadeActive = true;
            _fadeTimer  = 0f;
        }

        private void CompleteFade()
        {
            // Promote pending → active so the per-frame sync takes over with the new texture
            if (_pendingKBTexture != null)
            {
                _activeKBTexture   = _pendingKBTexture;
                _activeCtrlTexture = _pendingCtrlTexture;
            }
            // Commit the new texture to the main spellbook sprite
            if (_spellbookUIID != 0 && _activeKBTexture != null)
            {
                bool ctrl = Input.inputMode == InputMode.Controller;
                Bridge.SetTexturePath(_spellbookUIID, ctrl ? _activeCtrlTexture! : _activeKBTexture!);
            }
            // Reset SpellbookNew to invisible / transparent
            if (_spellbookNewID != 0)
            {
                Bridge.SetVisibility(_spellbookNewID, false);
                Bridge.SetColor(_spellbookNewID, 1f, 1f, 1f, 0f);
            }
            _pendingKBTexture   = null;
            _pendingCtrlTexture = null;
            _fadeActive         = false;
            _fadeTimer          = 0f;
        }

        // ── Back-button events ───────────────────────────────────────────────
        public void _when_spellbookbackbutton_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("SpellbookBackButton"), "UI-Back");
            Bridge.SetVisibility(_spellbookUIID, false);
            SetChildButtonsActive(_spellbookUIID, false);

            if (_fadeActive) CompleteFade();

            PauseManager.SetSpellbook(false);  // unpauses audio via Sync()
            Bridge.SetGroupPaused("SFX", false);
            if (_spellbookAudio != null) { _spellbookAudio.sound = "Spellbook_Close_01"; _spellbookAudio.play(); }
        }

        public void _when_spellbookbackbutton_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("SpellbookBackButton"), "UI-Back-Selected");
        }

        // ── Utilities ────────────────────────────────────────────────────────
        private void SetChildButtonsActive(uint parentID, bool active)
        {
            uint count = Bridge.GetChildCount(parentID);
            for (uint i = 0; i < count; i++)
            {
                uint childID = Bridge.GetChild(parentID, i);
                if (Bridge.GetNodeType(childID) == (uint)NodeType.BUTTON)
                    Bridge.SetButtonActive(childID, active);
                SetChildButtonsActive(childID, active);
            }
        }

        public void Default_enter()  {}
        public void Default_update() {}
        public void Default_exit()   {}
    }
}
