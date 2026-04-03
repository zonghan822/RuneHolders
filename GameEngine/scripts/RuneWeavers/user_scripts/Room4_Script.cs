namespace Eno
{
    public class Room4_Script : ScriptBase
    {
        private static Room4_Script? _instance;

        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;

        private bool  _scrollPickedUp = false;
        private bool  _wavesStarted   = false;
        private bool  _roomCleared    = false;

        private uint  _uiInfoID       = 0;
        private float _uiInfoTimer    = 0f;
        private float _lockRumbleTimer = 0f;
        private uint  _scrollPopUpID  = 0;
        private bool  _prevController = false;
        private int   _tutorialStage = 0;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance = this;
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room4_Scroll");
            _exitDoorID  = Bridge.GetNode("Door4to5");
            _spellbookID = Bridge.GetNode("Spellbook");
            _uiInfoID    = Bridge.GetNode("UI_Info");

            _scrollPopUpID  = Bridge.GetNode("Room4_ScrollPopUp");
            _prevController = Input.inputMode == InputMode.Controller;
            if (_scrollPopUpID != 0)
                Bridge.SetTexturePath(_scrollPopUpID, _prevController ? "UI-PopUp-A" : "UI-PopUp-E");

            _scrollPickedUp = false;
            _wavesStarted   = false;
            _roomCleared    = false;
            _uiInfoTimer    = 0f;
            _tutorialStage  = 0;
            DummyYellow_Script.WasHit = false;
            DummyRed_Script.WasHit    = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave1();
        }

        public static void CleanupDummies()
        {
            DummyYellow_Script.Cleanup();
            DummyRed_Script.Cleanup();
        }

        public void Update()
        {
            if (_lockRumbleTimer > 0f)
            {
                _lockRumbleTimer -= Time.deltaTime;
                if (_lockRumbleTimer <= 0f)
                    Input.setGamepadRumble(0f, 0f);
            }

            bool isController = Input.inputMode == InputMode.Controller;
            if (isController != _prevController)
            {
                _prevController = isController;
                if (_scrollPopUpID != 0)
                    Bridge.SetTexturePath(_scrollPopUpID, isController ? "UI-PopUp-A" : "UI-PopUp-E");
            }

            UpdateTutorialUI();

            // Check dummy clear condition
            if (_wavesStarted && !_roomCleared && DummyYellow_Script.WasHit)
            {
                _roomCleared = true;
                UnlockDoor("Door4to3");
                UnlockDoor("Door4to5");
            }

            // E on scroll -> pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[1] &&
                (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A)) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                if (_doorAudio != null) { _doorAudio.sound = "Spellbook_Open_01"; _doorAudio.play(); }
                RuneSystem.SpellUnlocked[1] = true; // Light
                // Hide the scroll
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                // Show spellcasting tutorial 3
                if (_uiInfoID != 0)
                {
                    Bridge.SetTexturePathAndResize(_uiInfoID, "Text-SpellcastingTutorial3");
                    Bridge.SetVisibility(_uiInfoID, true);
                }
                _tutorialStage = 1;
                // Update spellbook
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-2Spells");
            }
        }

        private void UpdateTutorialUI()
        {
            switch (_tutorialStage)
            {
                case 1:
                    if (PlayerController.SpellbookOpen)
                        _tutorialStage = 2;
                    break;

                case 2:
                    if (!PlayerController.SpellbookOpen)
                    {
                        if (_uiInfoID != 0)
                            Bridge.SetTexturePathAndResize(_uiInfoID, "Text-SpellcastingTutorial4");
                        _tutorialStage = 3;
                    }
                    break;

                case 3:
                    if (DummyYellow_Script.WasHit || DummyRed_Script.WasHit)
                    {
                        if (_uiInfoID != 0)
                            Bridge.SetTexturePathAndResize(_uiInfoID, "Text-SpellcastingTutorial4-Complete");
                        _uiInfoTimer   = 0f;
                        _tutorialStage = 4;
                    }
                    break;

                case 4:
                    _uiInfoTimer += Time.deltaTime;
                    if (_uiInfoTimer >= 3f)
                    {
                        if (_uiInfoID != 0)
                            Bridge.SetVisibility(_uiInfoID, false);
                        _tutorialStage = 5;
                    }
                    break;
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            LockDoor("Door4to3");
            LockDoor("Door4to5");
            SpawnPrefab("DummyRed",  830f, -3700f);
            SpawnPrefab("DummyYellow", 1080f, -3700f);
        }

        private void LockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
            {
                Bridge.ChangeState(id, "Locked");
                if (_doorAudio != null) { _doorAudio.sound = "Door_Close_01"; _doorAudio.play(); }
            }
            Input.setGamepadRumble(0.5f, 0.5f);
            _lockRumbleTimer = 0.2f;
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
            {
                Bridge.ChangeState(id, "Unlocking");
                if (_doorAudio != null) { _doorAudio.sound = "Door_Open_02"; _doorAudio.play(); }
            }
        }
    }
}
