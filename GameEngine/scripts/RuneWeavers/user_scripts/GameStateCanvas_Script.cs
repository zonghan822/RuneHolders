using System;
using System.Collections.Generic;

namespace Eno
{
	public class GameStateCanvas_Script : ScriptBase
    {

        private uint _winLoseCanvasID = 0;
        private uint _defeatScreenID  = 0;
        private uint _pauseScreenID   = 0;
        private uint _victoryScreenID = 0;
        private uint _areYouSureID    = 0;

        public enum prevScreen
        {
            WinScreen,
            LoseScreen,
            PauseScreen,
        };
        prevScreen _prevScreen;

        // ------------------------------------------------------------------ //
        // Controller navigation state
        // ------------------------------------------------------------------ //

        private enum PauseNav  { None, Resume, Quit, MainMenu }
        private enum QuitNav   { None, Yes, No }
        private enum EndNav    { None, Quit, MainMenu }

        private PauseNav _pauseSel  = PauseNav.None;
        private QuitNav  _quitSel   = QuitNav.None;
        private EndNav   _winSel    = EndNav.None;
        private EndNav   _loseSel   = EndNav.None;
        private bool     _prevPaused      = false;
        private bool     _prevWinVisible  = false;
        private bool     _prevLoseVisible = false;
        private bool     _stickLeft       = false;
        private bool     _stickRight      = false;
        private bool     _stickUp         = false;

        public void Begin()
        {
            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _defeatScreenID  = Bridge.GetNode("DefeatScreen");
            _pauseScreenID   = Bridge.GetNode("PauseScreen");
            _victoryScreenID = Bridge.GetNode("WinScreen");
            _areYouSureID    = Bridge.GetNode("AreYouSureScreen");
        }

        public void Update()
        {
            bool paused = PlayerController.IsGamePaused;

            // Auto-highlight Resume the moment the pause screen opens
            if (paused && !_prevPaused)
                SelectPause(PauseNav.Resume);
            _prevPaused = paused;

            // Left stick edge detection (trigger once per flick)
            const float STICK_THRESHOLD = 0.5f;
            Vector2 stick = Input.getLeftStick(0.2f);
            bool stickLeftNow  = stick.x < -STICK_THRESHOLD;
            bool stickRightNow = stick.x >  STICK_THRESHOLD;
            bool stickUpNow    = stick.y < -STICK_THRESHOLD;
            bool stickLeftEdge  = stickLeftNow  && !_stickLeft;
            bool stickRightEdge = stickRightNow && !_stickRight;
            bool stickUpEdge    = stickUpNow    && !_stickUp;
            _stickLeft  = stickLeftNow;
            _stickRight = stickRightNow;
            _stickUp    = stickUpNow;

            bool up      = Input.isKeyPressed(KeyCode.W) || Input.isGamepadButtonPressed(GamepadButton.DPadUp)    || stickUpEdge;
            bool left    = Input.isKeyPressed(KeyCode.A) || Input.isGamepadButtonPressed(GamepadButton.DPadLeft)  || stickLeftEdge;
            bool right   = Input.isKeyPressed(KeyCode.D) || Input.isGamepadButtonPressed(GamepadButton.DPadRight) || stickRightEdge;
            bool confirm = Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A);
            bool back    = Input.isGamepadButtonPressed(GamepadButton.B);
            bool areYouSureVisible = Bridge.GetVisibility(_areYouSureID);

            // Win screen controller navigation
            bool winVisible = Bridge.GetVisibility(_victoryScreenID);
            if (winVisible && !_prevWinVisible) SelectWin(EndNav.Quit);
            _prevWinVisible = winVisible;
            if (winVisible && !areYouSureVisible)
            {
                if (left)    SelectWin(EndNav.Quit);
                if (right)   SelectWin(EndNav.MainMenu);
                if (confirm) ConfirmWin();
            }

            // Lose screen controller navigation
            bool loseVisible = Bridge.GetVisibility(_defeatScreenID);
            if (loseVisible && !_prevLoseVisible) SelectLose(EndNav.Quit);
            _prevLoseVisible = loseVisible;
            if (loseVisible && !areYouSureVisible)
            {
                if (left)    SelectLose(EndNav.Quit);
                if (right)   SelectLose(EndNav.MainMenu);
                if (confirm) ConfirmLose();
            }

            // Quit-confirm sub-screen (must run before pause guard so it
            // works for both pause and lose/win screens)
            if (areYouSureVisible)
            {
                if (left)    SelectQuit(QuitNav.No);
                if (right)   SelectQuit(QuitNav.Yes);
                if (confirm) ConfirmQuit();
                if (back)    _when_areyousurescreenno_onclick();
                return;
            }

            if (!paused) return;

            // Main pause screen
            if (up)      SelectPause(PauseNav.Resume);
            if (left)    SelectPause(PauseNav.Quit);
            if (right)   SelectPause(PauseNav.MainMenu);
            if (confirm) ConfirmPause();
        }

        // ------------------------------------------------------------------ //
        // Pause screen navigation
        // ------------------------------------------------------------------ //

        private void SelectPause(PauseNav btn)
        {
            if (_pauseSel == btn) return;
            SetPauseSprite(_pauseSel, false);
            _pauseSel = btn;
            SetPauseSprite(_pauseSel, true);
        }

        private void SetPauseSprite(PauseNav btn, bool selected)
        {
            switch (btn)
            {
                case PauseNav.Resume:
                    Bridge.SetTexturePath(Bridge.GetNode("PauseScreenResumeButton"),
                        selected ? "UI-Resume-Selected" : "UI-Resume");
                    break;
                case PauseNav.Quit:
                    Bridge.SetTexturePath(Bridge.GetNode("PauseScreenQuit"),
                        selected ? "UI-QuitGame-Selected" : "UI-Quit");
                    break;
                case PauseNav.MainMenu:
                    Bridge.SetTexturePath(Bridge.GetNode("PauseScreenMainMenu"),
                        selected ? "UI-MainMenu-Selected" : "UI-MainMenu");
                    break;
            }
        }

        private void ConfirmPause()
        {
            switch (_pauseSel)
            {
                case PauseNav.Resume:   _when_pausescreenresumebutton_onclick(); break;
                case PauseNav.Quit:     _when_pausescreenquit_onclick();         break;
                case PauseNav.MainMenu: _when_pausescreenmainmenu_onclick();     break;
            }
        }

        // ------------------------------------------------------------------ //
        // Quit-confirm navigation
        // ------------------------------------------------------------------ //

        private void SelectQuit(QuitNav btn)
        {
            if (_quitSel == btn) return;
            SetQuitSprite(_quitSel, false);
            _quitSel = btn;
            SetQuitSprite(_quitSel, true);
        }

        private void SetQuitSprite(QuitNav btn, bool selected)
        {
            switch (btn)
            {
                case QuitNav.Yes:
                    Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenYes"),
                        selected ? "UI-Yes-Selected" : "UI-Yes");
                    break;
                case QuitNav.No:
                    Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenNo"),
                        selected ? "UI-No-Selected" : "UI-No");
                    break;
            }
        }

        private void ConfirmQuit()
        {
            switch (_quitSel)
            {
                case QuitNav.Yes: _when_areyousurescreenyes_onclick(); break;
                case QuitNav.No:  _when_areyousurescreenno_onclick();  break;
            }
        }

        // ------------------------------------------------------------------ //
        // Helpers
        // ------------------------------------------------------------------ //

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

        // ------------------------------------------------------------------ //
        // Pause screen buttons
        // ------------------------------------------------------------------ //

		public void _when_pausescreenresumebutton_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenResumeButton"), "UI-Resume-Selected");
        }

		public void _when_pausescreenresumebutton_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenResumeButton"), "UI-Resume");
            SetPauseSprite(_pauseSel, false);
            _pauseSel = PauseNav.None;
            Bridge.SetAudioPaused(false);
            Bridge.SetCanvasActive(_winLoseCanvasID, false);
            PlayerController.IsGamePaused = false;
        }

		public void _when_pausescreenquit_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenQuit"), "UI-QuitGame-Selected");
        }

		public void _when_pausescreenquit_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenQuit"), "UI-Quit");
            _prevScreen = prevScreen.PauseScreen;
            Bridge.SetVisibility(_pauseScreenID, false);
            SetChildButtonsActive(_pauseScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
            SelectQuit(QuitNav.No);
        }

		public void _when_pausescreenmainmenu_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenMainMenu"), "UI-MainMenu-Selected");
        }

		public void _when_pausescreenmainmenu_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("PauseScreenMainMenu"), "UI-MainMenu");
            SetPauseSprite(_pauseSel, false);
            _pauseSel = PauseNav.None;
            Bridge.SetAudioPaused(false);
            Bridge.ChangeScene("MainMenu.txt");
        }

        // ------------------------------------------------------------------ //
        // Are-you-sure screen buttons
        // ------------------------------------------------------------------ //

		public void _when_areyousurescreenyes_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenYes"), "UI-Yes-Selected");
        }

		public void _when_areyousurescreenyes_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenYes"), "UI-Yes");
            Window.requestClose();
        }

		public void _when_areyousurescreenno_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenNo"), "UI-No-Selected");
        }

		public void _when_areyousurescreenno_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("AreYouSureScreenNo"), "UI-No");
            SetQuitSprite(_quitSel, false);
            _quitSel = QuitNav.None;
            Bridge.SetVisibility(_areYouSureID, false);
            SetChildButtonsActive(_areYouSureID, false);
            switch (_prevScreen)
            {
                case prevScreen.WinScreen:
                    Bridge.SetVisibility(_victoryScreenID, true);
                    SetChildButtonsActive(_victoryScreenID, true);
                    break;
                case prevScreen.PauseScreen:
                    Bridge.SetVisibility(_pauseScreenID, true);
                    SetChildButtonsActive(_pauseScreenID, true);
                    SelectPause(PauseNav.Resume);
                    break;
                case prevScreen.LoseScreen:
                    Bridge.SetVisibility(_defeatScreenID, true);
                    SetChildButtonsActive(_defeatScreenID, true);
                    break;
            }
        }

        // ------------------------------------------------------------------ //
        // Win / Lose screen navigation
        // ------------------------------------------------------------------ //

        private void SelectWin(EndNav btn)
        {
            if (_winSel == btn) return;
            SetWinSprite(_winSel, false);
            _winSel = btn;
            SetWinSprite(_winSel, true);
        }

        private void SetWinSprite(EndNav btn, bool selected)
        {
            switch (btn)
            {
                case EndNav.Quit:
                    Bridge.SetTexturePath(Bridge.GetNode("WinScreenQuit"),
                        selected ? "UI-QuitGame-Selected" : "UI-Quit");
                    break;
                case EndNav.MainMenu:
                    Bridge.SetTexturePath(Bridge.GetNode("WinScreenMainMenu"),
                        selected ? "UI-MainMenu-Selected" : "UI-MainMenu");
                    break;
            }
        }

        private void ConfirmWin()
        {
            switch (_winSel)
            {
                case EndNav.Quit:     _when_winscreenquit_onclick();     break;
                case EndNav.MainMenu: _when_winscreenmainmenu_onclick(); break;
            }
        }

        private void SelectLose(EndNav btn)
        {
            if (_loseSel == btn) return;
            SetLoseSprite(_loseSel, false);
            _loseSel = btn;
            SetLoseSprite(_loseSel, true);
        }

        private void SetLoseSprite(EndNav btn, bool selected)
        {
            switch (btn)
            {
                case EndNav.Quit:
                    Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenQuit"),
                        selected ? "UI-QuitGame-Selected" : "UI-Quit");
                    break;
                case EndNav.MainMenu:
                    Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenMainMenu"),
                        selected ? "UI-MainMenu-Selected" : "UI-MainMenu");
                    break;
            }
        }

        private void ConfirmLose()
        {
            switch (_loseSel)
            {
                case EndNav.Quit:     _when_defeatscreenquit_onclick();     break;
                case EndNav.MainMenu: _when_defeatscreenmainmenu_onclick(); break;
            }
        }

        // ------------------------------------------------------------------ //
        // Win screen buttons
        // ------------------------------------------------------------------ //

		public void _when_winscreenquit_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("WinScreenQuit"), "UI-QuitGame-Selected");
        }

		public void _when_winscreenquit_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("WinScreenQuit"), "UI-Quit");
            _prevScreen = prevScreen.WinScreen;
            Bridge.SetVisibility(_victoryScreenID, false);
            SetChildButtonsActive(_victoryScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
            SelectQuit(QuitNav.No);
        }

		public void _when_winscreenmainmenu_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("WinScreenMainMenu"), "UI-MainMenu-Selected");
        }

		public void _when_winscreenmainmenu_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("WinScreenMainMenu"), "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }

        // ------------------------------------------------------------------ //
        // Defeat screen buttons
        // ------------------------------------------------------------------ //

		public void _when_defeatscreenquit_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenQuitGame"), "UI-QuitGame-Selected");
        }

		public void _when_defeatscreenquit_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenQuitGame"), "UI-Quit");
            _prevScreen = prevScreen.LoseScreen;
            Bridge.SetVisibility(_defeatScreenID, false);
            SetChildButtonsActive(_defeatScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
            SelectQuit(QuitNav.No);
        }

		public void _when_defeatscreenmainmenu_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenMainMenu"), "UI-MainMenu-Selected");
        }

		public void _when_defeatscreenmainmenu_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("DefeatScreenMainMenu"), "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }
	}
}
