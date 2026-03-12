using System;
using System.Collections.Generic;

namespace Eno
{
	public class Main_Menu_Script : ScriptBase
    {
        public void Begin()
        {

        }
        public void Update()
        {

        }

        /* -------------------------- MAIN MENU BUTTONS ----------------------- */
        // -------------- PLAY BUTTON -------------------
		public void _when_play_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Play Button");
            Bridge.SetTexturePath(buttonID, "UI-Play-Selected");
        }

		public void _when_play_button_onclick()
        {
            uint buttonID = Bridge.GetNode("Play Button");
            // Set to button default sprite
            Bridge.SetTexturePath(buttonID, "UI-Play");
            Bridge.ChangeScene("Level.txt");
        }
    
        // -------------- CONTROL BUTTON -------------------
		public void _when_control_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Control Button");
            Bridge.SetTexturePath(buttonID, "UI-Controls-Selected");
        }
	
		public void _when_control_button_onclick()
        {
            // Set to button default sprite
            uint buttonID = Bridge.GetNode("Control Button");
            Bridge.SetTexturePath(buttonID, "UI-Controls");
            // Set Control Menu Canvas Active
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, true);
        }
	
        // ---------------- QUIT BUTTON -------------------
		public void _when_quit_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Quit Button");
            Bridge.SetTexturePath(buttonID, "UI-QuitGame-Selected");
        }
	
		public void _when_quit_button_onclick()
        {
            // Set to button default sprite
            uint buttonID = Bridge.GetNode("Quit Button");
            Bridge.SetTexturePath(buttonID, "UI-Quit");
            // Set Quit Menu Canvas Active
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, true);
        }
	

        /* -------------------------- CONTROL MENU BUTTONS ----------------------- */

        // ---------- CONTROLS MENU BACK BUTTON -------------
		public void _when_control_menu_back_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Control Menu Back Button");
            Bridge.SetTexturePath(buttonID, "UI-Back-Selected");
        }
	
		public void _when_control_menu_back_button_onclick()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Control Menu Back Button");
            Bridge.SetTexturePath(buttonID, "UI-Back");
            // Set Control Menu Canvas Inactive
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, false);
        }

        /* -------------------------- QUIT MENU BUTTONS ----------------------- */
    
        // ---------- QUIT MENU YES BUTTON -------------
		public void _when_quit_menu_yes_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Quit Menu Yes Button");
            Bridge.SetTexturePath(buttonID, "UI-Yes-Selected");
        }
	
		public void _when_quit_menu_yes_button_onclick()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Quit Menu Yes Button");
            Bridge.SetTexturePath(buttonID, "UI-Yes");
            // Set Quit Menu Canvas Inactive
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
            // Exit Game
            Window.requestClose();

        }
	
        // ---------- QUIT MENU NO BUTTON -------------
		public void _when_quit_menu_no_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Quit Menu No Button");
            Bridge.SetTexturePath(buttonID, "UI-No-Selected");
        }
	
		public void _when_quit_menu_no_button_onclick()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Quit Menu No Button");
            Bridge.SetTexturePath(buttonID, "UI-No");
            // Set Quit Menu Canvas Inactive
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
        }
	}
}
			