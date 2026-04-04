namespace Eno
{
    public class Room8_Script : ScriptBase
    {
        private static Room8_Script? _instance;

        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;

        private bool  _postDialogueActivated = false;
        private bool  _scrollPickedUp        = false;
        private bool  _wavesStarted          = false;
        private bool  _roomCleared           = false;
        private bool  _wasEverCleared        = false;
        private bool  _doorsLocked           = false;

        private uint  _uiInfoID       = 0;
        private float _uiInfoTimer    = 0f;
        private int   _tutorialStage  = 0;
        private float _lockRumbleTimer = 0f;
        private uint  _scrollPopUpID  = 0;
        private bool  _prevController = false;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _instance = this;
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room8_Scroll");
            _exitDoorID  = Bridge.GetNode("Door8to9");
            _spellbookID = Bridge.GetNode("Spellbook");
            _scrollPopUpID  = Bridge.GetNode("Room8_ScrollPopUp");

            _prevController = Input.inputMode == InputMode.Controller;
            if (_scrollPopUpID != 0)
                Bridge.SetTexturePath(_scrollPopUpID, _prevController ? "UI-PopUp-A" : "UI-PopUp-E");

            _uiInfoID    = Bridge.GetNode("UI_Info");

            _postDialogueActivated = false;
            _scrollPickedUp        = false;
            _wavesStarted          = false;
            _roomCleared           = false;
            _wasEverCleared        = false;
            _doorsLocked           = false;
            _uiInfoTimer           = 0f;
            _tutorialStage         = 0;
            DummyBlue_Script.WasHit = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            DummyBlue_Script.WasHit = false;
            _instance.SpawnWave1();
        }

        public static void StopUI()
        {
            if (_instance == null) return;
            _instance._tutorialStage = 4;
            if (_instance._uiInfoID != 0)
                Bridge.SetVisibility(_instance._uiInfoID, false);
        }

        public static void CleanupDummies()
        {
            DummyRed_Script.Cleanup();
            DummyYellow_Script.Cleanup();
            DummyBlue_Script.Cleanup();
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
            if (_wavesStarted && !_roomCleared && (DummyBlue_Script.WasHit || GetAllEnemies().Count == 0))
            {
                _roomCleared    = true;
                _wasEverCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                if (_doorsLocked)
                {
                    _doorsLocked = false;
                    UnlockDoor("Door8to7");
                    UnlockDoor("Door8to9");
                }
            }

            // After dialogue ends: activate ui sprite, then hide after 3 seconds
            if (!_postDialogueActivated && Scroll_Dialouge_Script.DialogueShown[2])
            {
                _postDialogueActivated = true;
            }

            // E on scroll → pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[2] &&
                (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A)) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                if (_doorAudio != null) { _doorAudio.sound = "Spellbook_Open_01"; _doorAudio.play(); }
                RuneSystem.SpellUnlocked[2] = true; // Ice
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
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-3Spells");
            }
        }

        private void UpdateTutorialUI()
        {
            switch (_tutorialStage)
            {
                case 1:
                    _uiInfoTimer += Time.deltaTime;
                    if (PauseManager.SpellbookOpen || _uiInfoTimer >= 5f)
                    {
                        if (_uiInfoID != 0) Bridge.SetVisibility(_uiInfoID, false);
                        _tutorialStage++;
                        _uiInfoTimer = 0f;
                    }
                    break;

                case 2:
                    if (!PauseManager.SpellbookOpen)
                    {
                        if (_uiInfoID != 0)
                        {
                            Bridge.SetTexturePathAndResize(_uiInfoID, "Text-SpellcastingTutorial4");
                            Bridge.SetVisibility(_uiInfoID, true);
                        }
                        _tutorialStage++;
                    }
                    break;

                case 3:
                    if (DummyBlue_Script.WasHit)
                    {
                        _uiInfoTimer += Time.deltaTime;
                        if (_uiInfoID != 0 && _uiInfoTimer >= 2f)
                        {
                            Bridge.SetVisibility(_uiInfoID, false);
                            _tutorialStage++;
                            _uiInfoTimer = 0f;
                        }
                    }
                    break;
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            if (!_wasEverCleared)
            {
                _doorsLocked = true;
                BGMforLevel1_Script.SetLayer2Combat(true);
                LockDoor("Door8to7");
                LockDoor("Door8to9");
            }
            _roomCleared = false;
            SpawnPrefab("DummyRed",    6570f, -4750f);
            SpawnPrefab("DummyYellow", 6890f, -4750f);
            SpawnPrefab("DummyBlue",   6730f, -4640f);
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
