using System;
using System.Collections.Generic;

namespace Eno
{
	public class WinLoseCanvas_Script : ScriptBase
    {
        private uint lastScreenID = 0;
        private int _pendingAction = 0; // 0=none, 1=quit app, 2=go to main menu

        public void Begin()
        {

		}

        public void Update()
        {

		}

        /* -------------------------- WIN SCREEN BUTTONS ----------------------- */

        // -------------- MAIN MENU BUTTON -------------------
		public void _when_winscreenmainmenubutton_onpressed()
        {
            // Change main menu button
            uint buttonID = Bridge.GetNode("WinScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }

		public void _when_winscreenmainmenubutton_onclick()
        {
            uint buttonID = Bridge.GetNode("WinScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            // Hide win screen and show AreYouSure
            uint winScreenID = Bridge.GetNode("VictoryScreen");
            Bridge.SetVisibility(winScreenID, false);
            lastScreenID = winScreenID;
            _pendingAction = 2;
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, true);
        }



        /* -------------------------- ARE YOU SURE SCREEN BUTTONS ----------------------- */



        /* -------------------------- DEFEAT SCREEN BUTTONS ----------------------- */

        // -------------- RESUME BUTTON -------------------
		public void _when_defeatscreenresumebutton_onpressed()
        {
            // Change resume button
            uint buttonID = Bridge.GetNode("DefeatScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume-Selected");
        }

		public void _when_defeatscreenresumebutton_onclick()
        {
            // Change resume button
            uint buttonID = Bridge.GetNode("DefeatScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume");
            // Set win lose canvas to inactive
            uint winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            Bridge.SetCanvasActive(winLoseCanvasID, false);
        }

        // -------------- MAIN MENU BUTTON -------------------
		public void _when_defeatscreenmainmenubutton_onpressed()
        {
            // Change main menu button
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }

		public void _when_defeatscreenmainmenubutton_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            // Hide defeat screen and show AreYouSure
            uint defeatScreenID = Bridge.GetNode("DefeatScreen");
            Bridge.SetVisibility(defeatScreenID, false);
            lastScreenID = defeatScreenID;
            _pendingAction = 2;
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, true);
        }

	
		public void _when_winscreenquitbutton_onpressed()
        {
            // Change quit button
            uint buttonID = Bridge.GetNode("WinScreenQuitButton");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_winscreenquitbutton_onclick()
        {
            uint buttonID = Bridge.GetNode("WinScreenQuitButton");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            // Hide win screen and show AreYouSure
            uint winScreenID = Bridge.GetNode("VictoryScreen");
            Bridge.SetVisibility(winScreenID, false);
            lastScreenID = winScreenID;
            _pendingAction = 1;
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, true);
        }
	

	
		public void _when_defeatscreenquitbutton_onpressed()
        {
            // Change quit button
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGameButton");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_defeatscreenquitbutton_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGameButton");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            // Hide defeat screen and show AreYouSure
            uint defeatScreenID = Bridge.GetNode("DefeatScreen");
            Bridge.SetVisibility(defeatScreenID, false);
            lastScreenID = defeatScreenID;
            _pendingAction = 1;
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, true);

        }
	
		public void _when_areyousurenobutton_onpressed()
        {
            // Change no button
            uint buttonID = Bridge.GetNode("AreYouSureNoButton");
            Bridge.SetTexturePath(buttonID, "UI-No-Selected");
        }
	
		public void _when_areyousurenobutton_onclick()
        {
            uint buttonID = Bridge.GetNode("AreYouSureNoButton");
            Bridge.SetTexturePath(buttonID, "UI-No");
            // Hide AreYouSure and restore last screen
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, false);
            Bridge.SetVisibility(lastScreenID, true);
            _pendingAction = 0;
        }
	
		public void _when_areyousureyesbutton_onpressed()
        {
            // Change yes button
            uint buttonID = Bridge.GetNode("AreYouSureYesButton");
            Bridge.SetTexturePath(buttonID, "UI-Yes-Selected");
        }
	
		public void _when_areyousureyesbutton_onclick()
        {
            uint buttonID = Bridge.GetNode("AreYouSureYesButton");
            Bridge.SetTexturePath(buttonID, "UI-Yes");
            uint areYouSureID = Bridge.GetNode("AreYouSureScreen");
            Bridge.SetVisibility(areYouSureID, false);
            if (_pendingAction == 2)
                Bridge.ChangeScene("MainMenu.txt");
            else
                Window.requestClose();
            _pendingAction = 0;
        }
	}
}
