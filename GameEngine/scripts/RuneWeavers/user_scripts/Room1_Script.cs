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
                (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A)) &&
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
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-1Spell");
                if (_exitDoorID  != 0) Bridge.ChangeState(_exitDoorID, "Unlocking");
            }
        }
    }
}
