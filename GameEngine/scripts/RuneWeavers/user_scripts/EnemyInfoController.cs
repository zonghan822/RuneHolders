namespace Eno
{
    public class EnemyInfoController : ScriptBase
    {
        // Set by room scripts to control which texture is shown when the info panel opens
        public static string PopupKeyboardTexture   = "";
        public static string PopupControllerTexture = "";

        private uint _enemyInfoID        = 0;
        private uint _enemyInfoPopupID   = 0;
        private uint _enemyInfoPopupBgID = 0;
        private bool _enemyInfoOpen      = false;

        public void Begin()
        {
            _enemyInfoID        = Bridge.GetNode("EnemyInfo");
            _enemyInfoPopupID   = Bridge.GetNode("EnemyInfoPopUp");
            _enemyInfoPopupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
            _enemyInfoOpen      = false;
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            // E or A/X — close info panel
            if (_enemyInfoOpen && PauseManager.EnemyInfoOpen &&
                (Input.isKeyPressed(KeyCode.E) || (isController && (Input.isGamepadButtonPressed(GamepadButton.A) || Input.isGamepadButtonPressed(GamepadButton.X)))))
            {
                _enemyInfoOpen = false;
                if (_enemyInfoID      != 0) Bridge.SetVisibility(_enemyInfoID, false);
                if (_enemyInfoPopupID != 0)
                {
                    Bridge.SetVisibility(_enemyInfoPopupID, false);
                    Bridge.SetButtonActive(_enemyInfoPopupID, false);
                }
                PauseManager.SetEnemyInfo(false);
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
