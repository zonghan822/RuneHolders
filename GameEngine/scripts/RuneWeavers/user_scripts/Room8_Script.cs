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

        private uint  _uiInfoID    = 0;
        private float _uiInfoTimer = 0f;
        private float _lockRumbleTimer = 0f;

        public void Begin()
        {
            _instance = this;
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room8_Scroll");
            _exitDoorID  = Bridge.GetNode("Door8to9");
            _spellbookID = Bridge.GetNode("Spellbook");
            _uiInfoID    = Bridge.GetNode("UI_Info");

            _postDialogueActivated = false;
            _scrollPickedUp        = false;
            _wavesStarted          = false;
            _roomCleared           = false;
            _uiInfoTimer           = 0f;
            DummyBlue_Script.WasHit = false;
        }

        public static void StartWaves()
        {
            _instance?.SpawnWave1();
        }

        public static void CleanupDummies()
        {
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

            if (_uiInfoID != 0 && _uiInfoTimer < 3f)
            {
                _uiInfoTimer += Time.deltaTime;
                if (_uiInfoTimer >= 3f)
                    Bridge.SetVisibility(_uiInfoID, false);
            }

            // Check dummy clear condition
            if (_wavesStarted && !_roomCleared && DummyBlue_Script.WasHit)
            {
                _roomCleared = true;
                UnlockDoor("Door8to7");
                UnlockDoor("Door8to9");
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
                RuneSystem.SpellUnlocked[2] = true; // Ice
                // Hide the scroll
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                // Show F-to-open hint
                if (_uiInfoID != 0)
                {
                    Bridge.SetTexturePath(_uiInfoID, "UI-FtoOpen-Text");
                    Bridge.SetVisibility(_uiInfoID, true);
                }
                _uiInfoTimer = 0f;
                // Update spellbook
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-3Spells");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            LockDoor("Door8to7");
            LockDoor("Door8to9");
            SpawnPrefab("DummyBlue", 6570f, -4750f);
            SpawnPrefab("DummyBlue", 6890f, -4750f);
            SpawnPrefab("DummyBlue", 6730f, -4640f);
        }

        private void LockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Locked");
            Input.setGamepadRumble(0.5f, 0.5f);
            _lockRumbleTimer = 0.2f;
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocking");
        }
    }
}
