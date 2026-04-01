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
                uint popupID = Bridge.GetNode("EnemyInfoPopUp");
                if (popupID != 0 && popupID != uint.MaxValue)
                {
                    Bridge.SetTexturePath(popupID, "Enemy-Slug-Frozen-Sheet");
                    Bridge.SetVisibility(popupID, true);
                    Bridge.SetButtonActive(popupID, true);
                }
                uint popupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
                if (popupBgID != 0 && popupBgID != uint.MaxValue)
                    Bridge.SetVisibility(popupBgID, true);
                uint enemyInfoID = Bridge.GetNode("EnemyInfo");
                if (enemyInfoID != 0 && enemyInfoID != uint.MaxValue)
                    Bridge.SetTexturePath(enemyInfoID, "UI-EnemyPopUp-LavaSlug");
            }

            _instance.SpawnWave1();
        }

        public void Update()
        {
            if (_uiInfoID != 0 && _uiInfoTimer < 3f)
            {
                _uiInfoTimer += Time.deltaTime;
                if (_uiInfoTimer >= 3f)
                    Bridge.SetVisibility(_uiInfoID, false);
            }

            if (!_roomCleared && Input.isKeyPressed(KeyCode.F1))
            {
                _roomCleared = true;
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
                UnlockDoor("Door3to4");
                UnlockDoor("Door4to3");
                UnlockDoor("Door3to2");
                UnlockDoor("Door2to3");
            }
        }

        private void SpawnWave1()
        {
            _wavesStarted = true;
            LockDoor("Door2to3");
            LockDoor("Door3to2");
            LockDoor("Door3to4");
            LockDoor("Door4to3");
            SpawnPrefab("SlugRed", 955f, -2826f);
        }

        private void SpawnWave2()
        {
            SpawnPrefab("SlugRed",  410f, -2693f);
            SpawnPrefab("SlugRed", 1503f, -2693f);
        }

        private void LockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Locked");
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocking");
        }
    }
}
