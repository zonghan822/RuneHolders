using System;
using System.Collections.Generic;

namespace Eno
{
	public class WinLoseCanvas_Script : ScriptBase
    {
        public void Begin()
        {

		}

        public void Update()
        {

		}

        /* -------------------------- DEFEAT SCREEN BUTTONS ----------------------- */

        // -------------- RESUME BUTTON -------------------
		public void _when_defeatscreenresumebutton_onpressed()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume-Selected");
        }

		public void _when_defeatscreenresumebutton_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenResumeButton");
            Bridge.SetTexturePath(buttonID, "UI-Resume");
            uint winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            Bridge.SetCanvasActive(winLoseCanvasID, false);
        }

        // -------------- MAIN MENU BUTTON -------------------
		public void _when_defeatscreenmainmenubutton_onpressed()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu-Selected");
        }

		public void _when_defeatscreenmainmenubutton_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenMainMenuButton");
            Bridge.SetTexturePath(buttonID, "UI-MainMenu");
            Bridge.ChangeScene("MainMenu.txt");
        }

        // -------------- QUIT GAME BUTTON -------------------
		public void _when_defeatscreenquitgamebutton_onpressed()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGameButton");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }

		public void _when_defeatscreenquitgamebutton_onclick()
        {
            uint buttonID = Bridge.GetNode("DefeatScreenQuitGameButton");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            Window.requestClose();
        }
	
		public void _when_winscreenmainmenubutton_onpressed()
        {

        }
	
		public void _when_winscreenmainmenubutton_onclick()
        {

        }
	
		public void _when_winscreenquitbutton_onpressed()
        {
            uint buttonID = Bridge.GetNode("WinScreenQuitButton");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_winscreenquitbutton_onclick()
        {
            uint buttonID = Bridge.GetNode("WinScreenQuitButton");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            Window.requestClose();
        }
	
		public void _when_areyousureyesbutton_onpressed()
        {

        }
	
		public void _when_areyousureyesbutton_onclick()
        {

        }
	
		public void _when_areyousurenobutton_onpressed()
        {

        }
	
		public void _when_areyousurenobutton_onclick()
        {

        }
	}
}
			