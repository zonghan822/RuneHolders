namespace Eno
{
    public class Room1_Script : ScriptBase
    {
        private static Room1_Script? _instance;
        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;
        private uint _uiInfoID       = 0;
        private uint _scrollPopUpID  = 0;
        private bool  _dialogueTriggered     = false;
        //private bool  _postDialogueActivated = false;
        private bool  _scrollPickedUp        = false;
        private int   _uiStage = 0;
        private AudioNode? _doorAudio        = null;

        public void Begin()
        {
            _instance = this;
            _playerID       = Bridge.GetNode("Player");
            _scrollID       = Bridge.GetNode("Room1_Scroll");
            _exitDoorID     = Bridge.GetNode("Door1to2");
            _uiInfoID       = Bridge.GetNode("UI_Info");
            _spellbookID    = Bridge.GetNode("Spellbook");
            _dialogueTriggered     = false;
            //_postDialogueActivated = false;
            _scrollPickedUp        = false;
            _uiStage           = 0;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;

            _scrollPopUpID  = Bridge.GetNode("E_PopUp");

        }

        public void Update()
        {
            // Update input-mode-dependent textures every frame
            bool isController = Input.inputMode == InputMode.Controller;
            if (_scrollPopUpID != 0)
                Bridge.SetTexturePath(_scrollPopUpID, isController ? "UI-PopUp-A" : "UI-PopUp-E");
            if (_spellbookID != 0 && !SpellbookController.FadeActive && !SpellbookController.FadePending)
                Bridge.SetTexturePath(_spellbookID, _scrollPickedUp
                    ? (isController ? "UI-Spellbook-1Spell-Controller" : "UI-Spellbook-1Spell")
                    : (isController ? "UI-Spellbook-NoSpells-Controller" : "UI-SpellBook-NoSpells"));
            if (_uiInfoID != 0 && Bridge.GetVisibility(_uiInfoID))
            {
                if (_uiStage == 1)
                    Bridge.SetTexturePathAndResize(_uiInfoID, isController ? "UI-LJoystickToMove-Text" : "Text-WASDToMove");
                else if (_uiStage == 2)
                    Bridge.SetTexturePathAndResize(_uiInfoID, isController ? "UI-AToInteract-Text" : "Text-EToInteract");
                else if (_uiStage == 3)
                    Bridge.SetTexturePathAndResize(_uiInfoID, isController ? "UI-YToSpellbook-Text" : "Text-FSpellbook");
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

            // Stage 1: WASD hint — advance on movement
            if (_uiStage == 1)
            {
                bool moved = Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.A) ||
                             Input.isKeyHeld(KeyCode.S) || Input.isKeyHeld(KeyCode.D);
                if (!moved && Input.inputMode == InputMode.Controller)
                {
                    Vector2 stick = Input.getLeftStick(0.2f);
                    moved = stick.x != 0f || stick.y != 0f;
                }
                if (moved)
                    AdvanceUIStage();
            }
            // Stage 2: Scroll pickup — advance when picked up
            else if (_uiStage == 2 && _scrollPickedUp)
                AdvanceUIStage();
            // Stage 3: Spellbook hint — hide on spellbook open
            else if (_uiStage == 3 && PauseManager.SpellbookOpen)
                AdvanceUIStage();

            // Scroll pickup
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[0] &&
                (Input.isKeyPressed(KeyCode.E) || (Input.inputMode == InputMode.Controller && Input.isGamepadButtonPressed(GamepadButton.A))) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                RuneSystem.SpellUnlocked[0] = true;
                SpellbookController.SpawnOrbitParticle(0);
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                if (_exitDoorID  != 0)
                {
                    Bridge.ChangeState(_exitDoorID, "Unlocking");
                    if (_doorAudio != null) { _doorAudio.sound = "Door_Open_02"; _doorAudio.play(); }
                }
                SpellbookController.QueueFadeIn("UI-Spellbook-1Spell", "UI-Spellbook-1Spell-Controller");
            }
        }

        public static void StopUI()
        {
            if (_instance == null) return;
            _instance._uiStage = 4;
            if (_instance._uiInfoID != 0)
                Bridge.SetVisibility(_instance._uiInfoID, false);
            _instance._uiInfoID = 0;
        }

        private void AdvanceUIStage()
        {
            _uiStage++;

            if (_uiInfoID == 0) return;

            bool ctrl = Input.inputMode == InputMode.Controller;
            switch (_uiStage)
            {
                case 1:
                    Bridge.SetTexturePathAndResize(_uiInfoID, ctrl ? "UI-LJoystickToMove-Text" : "Text-WASDToMove");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                case 2:
                    Bridge.SetTexturePathAndResize(_uiInfoID, ctrl ? "UI-AToInteract-Text" : "Text-EToInteract");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                case 3:
                    Bridge.SetTexturePathAndResize(_uiInfoID, ctrl ? "UI-YToSpellbook-Text" : "Text-FSpellbook");
                    Bridge.SetVisibility(_uiInfoID, true);
                    break;
                default:
                    Bridge.SetVisibility(_uiInfoID, false);
                    _instance._uiInfoID = 0;
                    break;
            }
        }
    }
}
