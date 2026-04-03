namespace Eno
{
    public class PauseMenuController : ScriptBase
    {
        private uint _winLoseCanvasID = 0;
        private uint _pauseScreenID   = 0;
        private uint _blackBgID       = 0;

        public void Begin()
        {
            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _pauseScreenID   = Bridge.GetNode("PauseScreen");
            _blackBgID       = Bridge.GetNode("BlackBG");
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            if ((Input.isKeyPressed(KeyCode.Escape) || (isController && Input.isGamepadButtonPressed(GamepadButton.Start)))
                && !PlayerController.IsGameCompleted && !PauseManager.SpellbookOpen)
            {
                bool nowPaused = !PauseManager.PauseMenuOpen;
                PauseManager.SetPauseMenu(nowPaused);
                Bridge.SetCanvasActive(_winLoseCanvasID, nowPaused);
                Bridge.SetVisibility(_pauseScreenID, nowPaused);
                SetChildButtonsActive(_pauseScreenID, nowPaused);
                if (_blackBgID != 0) Bridge.SetVisibility(_blackBgID, nowPaused);
            }
        }

        private void SetChildButtonsActive(uint parentID, bool active)
        {
            uint count = Bridge.GetChildCount(parentID);
            for (uint i = 0; i < count; i++)
            {
                uint childID = Bridge.GetChild(parentID, i);
                if (Bridge.GetNodeType(childID) == (uint)NodeType.BUTTON)
                    Bridge.SetButtonActive(childID, active);
                SetChildButtonsActive(childID, active);
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
