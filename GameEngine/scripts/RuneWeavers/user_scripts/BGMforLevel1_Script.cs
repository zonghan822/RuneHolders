using System;

namespace Eno
{
    public class BGMforLevel1_Script : ScriptBase
    {
        // Volume ceilings (0-100)
        public float layer1Volume        = 70f;   // BGM_01_Layer1 — exploration (always on)
        public float layer1CombatVolume  = 20f;   // BGM_01_Layer1 — ducked during combat
        public float combatVolume        = 70f;   // BGM_01_Layer2 — combat
        public float creakVolume         = 70f;   // Ambience_Creaking_Wood — fades out in combat
        public float creakCombatVolume   = 10f;   // Ambience_Creaking_Wood — ducked during combat
        public float fadeDuration        = 2.0f;  // seconds for crossfade

        private static BGMforLevel1_Script? _instance;

        // Spawned audio nodes
        private AudioNode? _layer1;
        private AudioNode? _creak;

        // Fade state
        private float _fadeTimer   = 0f;
        private bool  _fading      = false;
        private float _l1FromVol, _l1ToVol;
        private float _l2FromVol, _l2ToVol;
        private float _creakFromVol, _creakToVol;

        public void Begin()
        {
            _instance = this;

            // Layer1 — always audible, no fade
            _layer1 = SpawnPrefab("DefaultBGMLoop", 0f, 0f) as AudioNode;
            if (_layer1 != null)
            {
                _layer1.volume = layer1Volume;
                _layer1.sound  = "BGM_01_Layer1_loop";
                _layer1.play();
            }

            // Layer2 — starts silent, combat rooms raise it
            Bridge.SetSound(ID, "BGM_01_Layer2_loop");
            Bridge.AudioPlay(ID);
            Bridge.SetAudioVolume(ID, 0f);

            // Ambience_Creaking_Wood — starts audible, fades out during combat
            _creak = SpawnPrefab("DefaultBGMLoop", 0f, 0f) as AudioNode;
            if (_creak != null)
            {
                _creak.volume = creakVolume;
                _creak.sound  = "Ambience_Creaking_Wood";
                _creak.play();
            }
        }

        public void Update()
        {
            if (!_fading) return;

            _fadeTimer += Bridge.GetDeltaTime();
            float t = Math.Min(_fadeTimer / fadeDuration, 1f);

            float l1    = _l1FromVol    + (_l1ToVol    - _l1FromVol)    * t;
            float l2    = _l2FromVol    + (_l2ToVol    - _l2FromVol)    * t;
            float creak = _creakFromVol + (_creakToVol - _creakFromVol) * t;

            if (_layer1 != null) Bridge.SetAudioVolume(_layer1.ID, l1);
            Bridge.SetAudioVolume(ID, l2);
            if (_creak != null) Bridge.SetAudioVolume(_creak.ID, creak);

            if (t >= 1f) _fading = false;
        }

        // Called when win/defeat screen appears — silences all layers immediately
        public static void StopAllBGM()
        {
            if (_instance == null) return;
            if (_instance._layer1 != null) _instance._layer1.stop();
            if (_instance._creak  != null) _instance._creak.stop();
            Bridge.AudioStop(_instance.ID);
            _instance._fading = false;
        }

        // Called on respawn after death — restarts exploration BGM
        public static void RestartBGM()
        {
            if (_instance == null) return;
            if (_instance._layer1 != null)
            {
                _instance._layer1.volume = _instance.layer1Volume;
                _instance._layer1.sound  = "BGM_01_Layer1_loop";
                _instance._layer1.play();
            }
            Bridge.SetSound(_instance.ID, "BGM_01_Layer2_loop");
            Bridge.AudioPlay(_instance.ID);
            Bridge.SetAudioVolume(_instance.ID, 0f);
            if (_instance._creak != null)
            {
                _instance._creak.volume = _instance.creakVolume;
                _instance._creak.sound  = "Ambience_Creaking_Wood";
                _instance._creak.play();
            }
            _instance._fading = false;
        }

        // Called by room scripts: true = combat active, false = room cleared
        public static void SetLayer2Combat(bool inCombat)
        {
            if (_instance == null) return;

            // Snapshot current live volumes as fade start points
            _instance._l1FromVol    = _instance._layer1 != null
                ? Bridge.GetAudioVolume(_instance._layer1.ID) : 0f;
            _instance._l2FromVol    = Bridge.GetAudioVolume(_instance.ID);
            _instance._creakFromVol = _instance._creak != null
                ? Bridge.GetAudioVolume(_instance._creak.ID) : 0f;

            _instance._l1ToVol    = inCombat ? _instance.layer1CombatVolume : _instance.layer1Volume;
            _instance._l2ToVol    = inCombat ? _instance.combatVolume       : 0f;
            _instance._creakToVol = inCombat ? _instance.creakCombatVolume  : _instance.creakVolume;

            _instance._fadeTimer = 0f;
            _instance._fading    = true;
        }
    }
}
