namespace Eno
{
    public class EnemyInfoController : ScriptBase
    {
        // Set by room scripts to control which texture is shown when the info panel opens
        public static string PopupKeyboardTexture   = "";
        public static string PopupControllerTexture = "";

        private static EnemyInfoController? _instance;

        private uint _enemyInfoID        = 0;
        private uint _enemyInfoPopupID   = 0;
        private bool _enemyInfoOpen      = false;
        private AudioNode? _sfxAudio     = null;

        public void Begin()
        {
            _instance           = this;
            _enemyInfoID        = Bridge.GetNode("EnemyInfo");
            _enemyInfoPopupID   = Bridge.GetNode("EnemyInfoPopUp");
            _enemyInfoOpen      = false;
            _sfxAudio           = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public static void HidePopup()
        {
            if (_instance == null) return;
            // Close info panel if open
            if (_instance._enemyInfoOpen)
            {
                _instance._enemyInfoOpen = false;
                if (_instance._enemyInfoID != 0) Bridge.SetVisibility(_instance._enemyInfoID, false);
                PauseManager.SetEnemyInfo(false);
            }
            // Hide popup button
            if (_instance._enemyInfoPopupID != 0)
            {
                Bridge.SetVisibility(_instance._enemyInfoPopupID, false);
                Bridge.SetButtonActive(_instance._enemyInfoPopupID, false);
            }
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            // E or A/X — close info panel
            if (_enemyInfoOpen && PauseManager.EnemyInfoOpen &&
                (Input.isKeyPressed(KeyCode.E) || (isController && (Input.isGamepadButtonPressed(GamepadButton.A) || Input.isGamepadButtonPressed(GamepadButton.X)))))
            {
                _enemyInfoOpen = false;
                if (_enemyInfoID != 0) Bridge.SetVisibility(_enemyInfoID, false);
                PauseManager.SetEnemyInfo(false);
                if (_sfxAudio != null) { _sfxAudio.sound = "Spellbook_Close_01"; _sfxAudio.play(); }
                return;
            }

            // T or X — open info panel (only when popup button is visible)
            if (!_enemyInfoOpen && _enemyInfoPopupID != 0 && Bridge.GetVisibility(_enemyInfoPopupID) &&
                !PauseManager.IsGamePaused &&
                (Input.isKeyPressed(KeyCode.T) || (isController && Input.isGamepadButtonPressed(GamepadButton.X))))
            {
                if (_enemyInfoID != 0)
                {
                    string tex = isController ? PopupControllerTexture : PopupKeyboardTexture;
                    if (!string.IsNullOrEmpty(tex))
                        Bridge.SetTexturePath(_enemyInfoID, tex);
                    Bridge.SetVisibility(_enemyInfoID, true);
                }
                _enemyInfoOpen = true;
                PauseManager.SetEnemyInfo(true);
                if (_sfxAudio != null) { _sfxAudio.sound = "Spellbook_Open_01"; _sfxAudio.play(); }
            }
        }
    
        public void Default_enter()
        {
            // TODO: Add enter logic for Default state
        }

        public void Default_update()
        {
            // TODO: Add update logic for Default state
        }

        public void Default_exit()
        {
            // TODO: Add exit logic for Default state
        }
    }
}
