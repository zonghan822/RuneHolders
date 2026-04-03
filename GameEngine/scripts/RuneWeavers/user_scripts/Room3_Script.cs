namespace Eno
{
    public class Room3_Script : ScriptBase
    {
        private const float Wave2Delay = 3f;

        private static Room3_Script? _instance;
        private static bool _popupActivated = false;

        private bool  _wavesStarted = false;
        private bool  _wave2Spawned = false;
        private bool  _roomCleared  = false;
        private float _wave2Timer   = 0f;

        private uint  _uiInfoID    = 0;
        private float _uiInfoTimer = 0f;
        private float _lockRumbleTimer = 0f;

        public void Begin()
        {
            _instance       = this;
            _popupActivated = false;
            _wavesStarted   = false;
            _wave2Spawned   = false;
            _roomCleared    = false;
            _wave2Timer     = 0f;
            _uiInfoTimer    = 0f;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;

            _instance._uiInfoID = Bridge.GetNode("UI_Info");
            if (_instance._uiInfoID != 0)
            {
                Bridge.SetTexturePath(_instance._uiInfoID, "UI-MonsterColourInstructions-Text");
                Bridge.SetVisibility(_instance._uiInfoID, true);
            }
            _instance._uiInfoTimer = 0f;

            if (!_popupActivated)
            {
                _popupActivated = true;
                PlayerController.EnemyPopupKeyboardTexture   = "UI-EnemyPopUp-LavaSlug";
                PlayerController.EnemyPopupControllerTexture = "UI-EnemyPopUp-Slug-Controller";

                uint popupID = Bridge.GetNode("EnemyInfoPopUp");
                if (popupID != 0 && popupID != uint.MaxValue)
                {
                    string icon = Input.isGamepadConnected() ? "UI-EnemyIcon-Slug-Controller" : "UI-EnemyIcon-Slug";
                    Bridge.SetTexturePath(popupID, icon);
                    Bridge.SetVisibility(popupID, true);
                    Bridge.SetButtonActive(popupID, true);
                }
            }

            _instance.SpawnWave1();
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

            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door3to4");
                UnlockDoor("Door4to3");
                UnlockDoor("Door3to2");
                UnlockDoor("Door2to3");
            }

            if (!_wavesStarted || _roomCleared) return;

            if (!_wave2Spawned)
            {
                _wave2Timer += Time.deltaTime;
                if (_wave2Timer >= Wave2Delay)
                {
                    _wave2Spawned = true;
                    SpawnWave2();
                }
                return;
            }

            if (GetAllEnemies().Count == 0)
            {
                _roomCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                UnlockDoor("Door3to4");
                UnlockDoor("Door4to3");
                UnlockDoor("Door3to2");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            BGMforLevel1_Script.SetLayer2Combat(true);
            LockDoor("Door2to3");
            LockDoor("Door3to2");
            LockDoor("Door3to4");
            LockDoor("Door4to3");
            SpawnPrefab("SlugRed", 960f, -2850f);
        }

        private void SpawnWave2()
        {
            SpawnPrefab("SlugRed",  500f, -2635f);
            SpawnPrefab("SlugRed", 1418f, -2769f);
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
