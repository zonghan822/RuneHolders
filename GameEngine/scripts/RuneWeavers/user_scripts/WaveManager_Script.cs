namespace Eno
{
    public class WaveManager_Script : ScriptBase
    {
        private const float WaveDuration = 20f;

        private int   _currentWave = 0;
        private float _waveTimer   = 0f;
        private bool  _allWavesDone = false;

        // Wave definitions: each entry is an array of (prefabName, x, y) tuples
        private static readonly (string prefab, float x, float y)[][] Waves =
        {
            // Wave 1
            new[]
            {
                ("SlugRed", 952f, -2842f),
            },
            // Wave 2
            new[]
            {
                ("SlugRed",  407f, -2670f),
                ("SlugRed", 1500f, -2663f),
            },
        };

        public void Begin()
        {
            SpawnWave(_currentWave);
        }

        public void Update()
        {
            if (PlayerController.IsGamePaused) return;
            if (_allWavesDone) return;

            _waveTimer += Time.deltaTime;

            if (_waveTimer >= WaveDuration)
            {
                _waveTimer = 0f;
                _currentWave++;

                if (_currentWave < Waves.Length)
                {
                    SpawnWave(_currentWave);
                }
                else
                {
                    _allWavesDone = true;
                }
            }
        }

        private void SpawnWave(int waveIndex)
        {
            var wave = Waves[waveIndex];
            foreach (var (prefab, x, y) in wave)
            {
                SpawnPrefab(prefab, x, y);
            }
        }
    }
}
