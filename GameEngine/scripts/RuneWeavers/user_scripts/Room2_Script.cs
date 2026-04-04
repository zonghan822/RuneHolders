namespace Eno
{
    public class Room2_Script : ScriptBase
    {
        private static Room2_Script? _instance;
        private float _lockRumbleTimer = 0f;
        private bool  _roomCleared    = false;
        private bool  _wasEverCleared = false;
        private bool  _wavesStarted   = false;
        private bool  _doorsLocked    = false;
        private AudioNode? _doorAudio = null;

        private Sprite2D? _uiInfo        = null;
        private bool      _uiInfoShown   = false;
        private float     _uiInfoTimer   = 0f;
        // 0 = waiting for first spell cast
        // 1 = "cast complete" — 2 s countdown before showing part 2
        // 2 = waiting for dummy hit
        // 3 = "hit complete" — 3 s countdown before hiding
        // 4 = done
        private int       _tutorialStage = 0;

        public void Begin()
        {
            _instance     = this;
            _roomCleared  = false;
            _wavesStarted = false;
            _wasEverCleared = false;
            _doorsLocked    = false;
            DummyRed_Script.WasHit       = false;
            RuneSystem.SpellCastOccurred = false;

            _uiInfo        = null;
            _uiInfoShown   = false;
            _tutorialStage = 0;
            _uiInfoTimer   = 0f;

            try { _uiInfo = GetNode<Sprite2D>("UI_Info"); } catch { }
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void StartWaves()
        {
            if (_instance == null) return;
            if (_instance._wasEverCleared)
            {
                // Revisit — skip tutorial, spawn dummy directly
                DummyRed_Script.WasHit = false;
                _instance._roomCleared = false;
                _instance.SpawnWave1();
            }
            else
            {
                _instance.ActivateTutorialUI();
            }
        }

        private void ActivateTutorialUI()
        {
            if (_uiInfo == null)
                try { _uiInfo = GetNode<Sprite2D>("UI_Info"); } catch { }

            if (_uiInfo != null)
            {
                _uiInfo.setTextureAndResize(Input.inputMode == InputMode.Controller
                    ? "UI-ControllerCastingInstructions-Text"
                    : "Text-SpellcastingTutorial");
                _uiInfo.visible = true;
                _uiInfoShown    = true;
            }
            LockDoor("Door2to1");
            LockDoor("Door2to3");
        }

        public static void CleanupDummies()
        {
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

            UpdateTutorialUI();

            if (!_wavesStarted || _roomCleared) return;
            if (DummyRed_Script.WasHit || GetAllEnemies().Count == 0)
            {
                _roomCleared    = true;
                _wasEverCleared = true;
                BGMforLevel1_Script.SetLayer2Combat(false);
                if (_doorsLocked)
                {
                    _doorsLocked = false;
                    UnlockDoor("Door2to1");
                    UnlockDoor("Door2to3");
                }
            }
        }

        private void UpdateTutorialUI()
        {
            if (_uiInfo == null || !_uiInfoShown) return;

            switch (_tutorialStage)
            {
                case 0:
                    _uiInfoTimer += Time.deltaTime;
                    if (RuneSystem.SpellCastOccurred || _uiInfoTimer >= 10f)
                    {
                        RuneSystem.SpellCastOccurred = false;
                        _uiInfo.setTextureAndResize("Text-SpellcastingTutorial2");
                        _uiInfoTimer   = 0f;
                        _tutorialStage = 1;
                    }
                    break;

                case 1:
                    _uiInfoTimer += Time.deltaTime;
                    if (_uiInfoTimer >= 3f)
                    {
                        // Spawn target dummy after casting spell for the first time
                        _instance.SpawnWave1();
                        _uiInfo.setTextureAndResize("Text-SpellcastingTutorial2");
                        _uiInfoTimer   = 0f;
                        _tutorialStage = 2;
                    }
                    break;

                case 2:
                    if (DummyRed_Script.WasHit)
                    {
                        _uiInfo.visible = false;
                        _tutorialStage  = 3; // done — prevent re-triggering
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
            }
            _roomCleared = false;
            SpawnPrefab("DummyRed", 972f, -1700f);
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
