using System;
using System.Collections.Generic;

namespace Eno
{
	public class GameStateCanvas_Script : ScriptBase
    {

        private uint _winLoseCanvasID = 0;
        private uint _defeatScreenID  = 0;
        private uint _pauseScreenID = 0;
        private uint _victoryScreenID = 0;
        private uint _areYouSureID    = 0;

        public enum prevScreen
        {
            WinScreen,
            LoseScreen,
            PauseScreen,
            
        };
        prevScreen _prevScreen;

        public void Begin()
        {
            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _defeatScreenID  = Bridge.GetNode("DefeatScreen");
            _pauseScreenID = Bridge.GetNode("PauseScreen");
            _victoryScreenID = Bridge.GetNode("WinScreen");
            _areYouSureID = Bridge.GetNode("AreYouSureScreen");
		}

        public void Update()
        {

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

	
		public void _when_pausescreenresumebutton_onpressed()
        {
            uint buttonID = Bridge.GetNode("PauseScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume-Selected");
        }
	
		public void _when_pausescreenresumebutton_onclick()
        {
            uint buttonID = Bridge.GetNode("PauseScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume");
            Bridge.SetCanvasActive(_winLoseCanvasID, false);
            PlayerController.IsGamePaused = false;
        }
	
		public void _when_pausescreenquit_onpressed()
        {
            uint buttonID = Bridge.GetNode("PauseScreenQuit");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_pausescreenquit_onclick()
        {
            uint buttonID = Bridge.GetNode("PauseScreenQuit");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            _prevScreen = prevScreen.PauseScreen;
            Bridge.SetVisibility(_pauseScreenID, false);
            SetChildButtonsActive(_pauseScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
        }
	
		public void _when_pausescreenmainmenu_onpressed()
        {
            uint buttonID = Bridge.GetNode("PauseScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }
	
		public void _when_pausescreenmainmenu_onclick()
        {
            uint buttonID = Bridge.GetNode("PauseScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }
	
		public void _when_areyousurescreenyes_onpressed()
        {
            uint buttonID = Bridge.GetNode("AreYouSureScreenYes");
            Bridge.SetTexturePath(buttonID, "UI-Yes-Selected");
        }
	
		public void _when_areyousurescreenyes_onclick()
        {
            uint buttonID = Bridge.GetNode("AreYouSureScreenYes");
            Bridge.SetTexturePath(buttonID, "UI-Yes");
            Window.requestClose();
        }
	
		public void _when_areyousurescreenno_onpressed()
        {
            uint buttonID = Bridge.GetNode("AreYouSureScreenNo");
            Bridge.SetTexturePath(buttonID, "UI-No-Selected");
        }
	
		public void _when_areyousurescreenno_onclick()
        {
            uint buttonID = Bridge.GetNode("AreYouSureScreenNo");
            Bridge.SetTexturePath(buttonID, "UI-No");
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
                break;
                case prevScreen.LoseScreen:
                Bridge.SetVisibility(_defeatScreenID, true);
                SetChildButtonsActive(_defeatScreenID, true);
                break;
            }

        }
	
		public void _when_winscreenquit_onpressed()
        {
            uint buttonID = Bridge.GetNode("WinScreenQuit");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_winscreenquit_onclick()
        {
            uint buttonID = Bridge.GetNode("WinScreenQuit");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            _prevScreen = prevScreen.WinScreen;
            Bridge.SetVisibility(_victoryScreenID, false);
            SetChildButtonsActive(_victoryScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
        }
	
		public void _when_winscreenmainmenu_onpressed()
        {
            uint buttonID = Bridge.GetNode("WinScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }
	
		public void _when_winscreenmainmenu_onclick()
        {
            uint buttonID = Bridge.GetNode("WinScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }
	
		public void _when_defeatscreenquit_onpressed()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGame");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_defeatscreenquit_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGame");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            _prevScreen = prevScreen.LoseScreen;
            Bridge.SetVisibility(_defeatScreenID, false);
            SetChildButtonsActive(_defeatScreenID, false);
            Bridge.SetVisibility(_areYouSureID, true);
            SetChildButtonsActive(_areYouSureID, true);
        }
	
		public void _when_defeatscreenmainmenu_onpressed()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }
	
		public void _when_defeatscreenmainmenu_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenu");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }
	}
}
			