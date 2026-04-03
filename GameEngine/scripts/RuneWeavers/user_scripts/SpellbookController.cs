namespace Eno
{
    public class SpellbookController : ScriptBase
    {
        private uint       _spellbookUIID  = 0;
        private AudioNode? _spellbookAudio = null;

        public void Begin()
        {
            _spellbookUIID  = Bridge.GetNode("Spellbook");
            _spellbookAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            if (_spellbookAudio != null) _spellbookAudio.group = "BGM";
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            if ((Input.isKeyPressed(KeyCode.F) || (isController && Input.isGamepadButtonPressed(GamepadButton.Y)))
                && _spellbookUIID != 0
                && !Scroll_Dialouge_Script.DialogueActive
                && !PauseManager.PauseMenuOpen)
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
                }
                else
                {
                    // Unpause audio first, then play close sound
                    PauseManager.SetSpellbook(false);
                    Bridge.SetGroupPaused("SFX", false);
                    if (_spellbookAudio != null) { _spellbookAudio.sound = "Spellbook_Close_01"; _spellbookAudio.play(); }
                }
            }
        }

        public void _when_spellbookbackbutton_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("SpellbookBackButton"), "UI-Back");
            Bridge.SetVisibility(_spellbookUIID, false);
            SetChildButtonsActive(_spellbookUIID, false);
            PauseManager.SetSpellbook(false);  // unpauses audio via Sync()
            Bridge.SetGroupPaused("SFX", false);
            if (_spellbookAudio != null) { _spellbookAudio.sound = "Spellbook_Close_01"; _spellbookAudio.play(); }
        }

        public void _when_spellbookbackbutton_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("SpellbookBackButton"), "UI-Back-Selected");
        }

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
    
        public void Default_enter()
        {
            // TODO: Add enter logic for Default state
        }

        public void Default_update()
        {
            // TODO: Add update logic for Default state
        }

        public void Default_exit()
        {
            // TODO: Add exit logic for Default state
        }
    }
}
