namespace Eno
{
    public class Room1_Script : ScriptBase
    {
        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;
        private uint _uiInfoID       = 0;
        private uint _scrollPopUpID  = 0;
        private bool _prevController = false;
        private bool  _dialogueTriggered     = false;
        //private bool  _postDialogueActivated = false;
        private bool  _scrollPickedUp        = false;
        private int   _uiStage = 0;
        private float _uiTimer           = 0f;
        private AudioNode? _doorAudio        = null;

        public void Begin()
        {
            _playerID       = Bridge.GetNode("Player");
            _scrollID       = Bridge.GetNode("Room1_Scroll");
            _exitDoorID     = Bridge.GetNode("Door1to2");
            _uiInfoID       = Bridge.GetNode("UI_Info");
            _spellbookID    = Bridge.GetNode("Spellbook");
            _dialogueTriggered     = false;
            //_postDialogueActivated = false;
            _scrollPickedUp        = false;
            _uiStage           = 0;
            _uiTimer           = 0f;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;

            _scrollPopUpID  = Bridge.GetNode("Room1_ScrollPopUp");
            _prevController = Input.inputMode == InputMode.Controller;
            if (_scrollPopUpID != 0) Bridge.SetTexturePath(_scrollPopUpID, _prevController ? "UI-PopUp-A" : "UI-PopUp-E");

        }

        public void Update()
        {
            // Swap scroll popup texture on input mode change
            bool isController = Input.inputMode == InputMode.Controller;
            if (isController != _prevController)
            {
                _prevController = isController;
                if (_scrollPopUpID != 0)
                    Bridge.SetTexturePath(_scrollPopUpID, isController ? "UI-PopUp-A" : "UI-PopUp-E");
            }

            // Trigger Room 1 dialogue — retry each frame until instance is ready
            if (!_dialogueTriggered)
            {
                Scroll_Dialouge_Script.TriggerDialogue(0);
                if (Scroll_Dialouge_Script.DialogueActive || Scroll_Dialouge_Script.DialogueShown[0])
                    _dialogueTriggered = true;
            }

            // After dialogue ends: begin UI sequence
            if (_uiStage == 0 && Scroll_Dialouge_Script.DialogueShown[0])
                AdvanceUIStage();

            if (_uiStage > 0 && _uiStage < 4)
                _uiTimer += Time.deltaTime;

            // Stage 1: WASD hint — advance on input or after 3 s
            if (_uiStage == 1)
            {
                bool moved = Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.A) ||
                             Input.isKeyHeld(KeyCode.S) || Input.isKeyHeld(KeyCode.D);
                if (!moved && Input.inputMode == InputMode.Controller)
                {
                    Vector2 stick = Input.getLeftStick(0.2f);
                    moved = stick.x != 0f || stick.y != 0f;
                }
                if (moved || _uiTimer >= 3f)
                    AdvanceUIStage();
            }

            // Stage 2: WASD complete — advance on scroll pickup or after 3 s
            if (_uiStage == 2 && (_scrollPickedUp || _uiTimer >= 3f))
                AdvanceUIStage();

            // Stage 3: FtoOpen — hide after 3 s
            if (_uiStage == 3 && _uiTimer >= 3f)
                AdvanceUIStage();

            // Scroll pickup
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[0] &&
                (Input.isKeyPressed(KeyCode.E) || (Input.inputMode == InputMode.Controller && Input.isGamepadButtonPressed(GamepadButton.A))) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                AdvanceUIStage();
                RuneSystem.SpellUnlocked[0] = true;
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

        private void AdvanceUIStage()
        {
            _uiStage++;
            _uiTimer = 0f;

            if (_uiInfoID == 0) return;

            bool ctrl = Input.inputMode == InputMode.Controller;
            switch (_uiStage)
            {
                case 1:
                    Bridge.SetTexturePathAndResize(_uiInfoID, ctrl ? "UI-LJoystickToMove-Text" : "UI-WASD-Text");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                case 2:
                    Bridge.SetTexturePathAndResize(_uiInfoID, "Text-WASDToMove-Complete");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                case 3:
                    Bridge.SetTexturePathAndResize(_uiInfoID, ctrl ? "UI-YToSpellbook" : "Text-FSpellbook");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                default:
                    Bridge.SetVisibility(_uiInfoID, false);
                    break;
            }
        }
    }
}
