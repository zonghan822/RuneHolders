namespace Eno
{
    public static class PauseManager
    {
        private static bool _pauseMenu     = false;
        private static bool _spellbook     = false;
        private static bool _enemyInfo     = false;
        private static bool _cameraPanning = false;

        public static bool IsGamePaused   => _pauseMenu || _spellbook || _enemyInfo || _cameraPanning;
        public static bool IsMenuPaused   => _pauseMenu || _spellbook || _enemyInfo;

        public static bool PauseMenuOpen   => _pauseMenu;
        public static bool SpellbookOpen   => _spellbook;
        public static bool EnemyInfoOpen   => _enemyInfo;
        public static bool CameraPanning   => _cameraPanning;

        public static void SetPauseMenu(bool active)     { _pauseMenu     = active; Sync(); }
        public static void SetSpellbook(bool active)     { _spellbook     = active; Sync(); }
        public static void SetEnemyInfo(bool active)     { _enemyInfo     = active; Sync(); }
        public static void SetCameraPanning(bool active) { _cameraPanning = active; }

        public static void Reset()
        {
            _pauseMenu     = false;
            _spellbook     = false;
            _enemyInfo     = false;
            _cameraPanning = false;
            Sync();
        }

        private static void Sync()
        {
            // Spellbook-only pause keeps BGM playing — SpellbookController handles SFX group directly.
            // Only pause all audio when the pause menu, enemy info, or camera pan is active.
            Bridge.SetAudioPaused(_pauseMenu || _enemyInfo || _cameraPanning);
        }
    }
}
