namespace Eno
{
    public class Room1_Script : ScriptBase
    {
        private uint _scrollID    = 0;
        private uint _playerID   = 0;
        private uint _spellbookID = 0;  
        private uint _exitDoorID  = 0;
        private uint _uiInfoID    = 0;

        private bool  _dialogueTriggered     = false;
        private bool  _postDialogueActivated = false;
        private bool  _scrollPickedUp        = false;
        private float _uiInfoTimer           = 0f;
        private AudioNode? _doorAudio        = null;

        public void Begin()
        {
            _playerID       = Bridge.GetNode("Player");
            _scrollID       = Bridge.GetNode("Room1_Scroll");
            _exitDoorID     = Bridge.GetNode("Door1to2");
            _uiInfoID       = Bridge.GetNode("UI_Info");
            _spellbookID    = Bridge.GetNode("Spellbook");
            _dialogueTriggered     = false;
            _postDialogueActivated = false;
            _scrollPickedUp        = false;
            _uiInfoTimer           = 0f;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;

            // Room 1 has no enemies — hide the enemy popup and clear its textures
            uint popupID   = Bridge.GetNode("EnemyInfoPopUp");
            uint popupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
            if (popupID   != 0 && popupID   != uint.MaxValue) Bridge.SetVisibility(popupID,   false);
            if (popupBgID != 0 && popupBgID != uint.MaxValue) Bridge.SetVisibility(popupBgID, false);
            PlayerController.EnemyPopupKeyboardTexture   = "";
            PlayerController.EnemyPopupControllerTexture = "";
        }

        public void Update()
        {
            // Trigger Room 1 dialogue — retry each frame until instance is ready
            if (!_dialogueTriggered)
            {
                Scroll_Dialouge_Script.TriggerDialogue(0);
                if (Scroll_Dialouge_Script.DialogueActive || Scroll_Dialouge_Script.DialogueShown[0])
                    _dialogueTriggered = true;
            }

            // After dialogue ends: activate ui sprite, then hide after 3 seconds
            if (!_postDialogueActivated && Scroll_Dialouge_Script.DialogueShown[0])
            {
                _postDialogueActivated = true;

                if (_uiInfoID != 0) 
                {
                    Bridge.SetTexturePath(_uiInfoID, "UI-WASD-Text");
                    Bridge.SetVisibility(_uiInfoID, true);
                }
            }

            // Set a timer to hide the UI after 3 seconds
            if (_postDialogueActivated && _uiInfoTimer < 3f)
            {
                _uiInfoTimer += Time.deltaTime;
                if (_uiInfoTimer >= 3f && _uiInfoID != 0)
                    Bridge.SetVisibility(_uiInfoID, false);
            }

            // E on scroll → pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[0] &&
                (Input.isKeyPressed(KeyCode.E) || (Input.inputMode == InputMode.Controller && Input.isGamepadButtonPressed(GamepadButton.A))) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                RuneSystem.SpellUnlocked[0] = true; // Fire
                // Press f to open spell book
                // Swap UI_Info texture and reset its timer
                if (_uiInfoID != 0)
                {
                    Bridge.SetTexturePath(_uiInfoID, "UI-FtoOpen-Text");
                    Bridge.SetVisibility(_uiInfoID, true);
                }
                _uiInfoTimer = 0f;
                // Hide the scroll
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                if (_doorAudio != null) { _doorAudio.sound = "Spellbook_Open_01"; _doorAudio.play(); }
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-1Spell");
                if (_exitDoorID  != 0)
                {
                    Bridge.ChangeState(_exitDoorID, "Unlocking");
                    if (_doorAudio != null) { _doorAudio.sound = "Door_Open_02"; _doorAudio.play(); }
                }
            }
        }
    }
}
