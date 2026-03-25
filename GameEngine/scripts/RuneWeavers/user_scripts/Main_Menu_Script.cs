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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Control Menu Canvas Active
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, true);
            SetChildButtonsActive(controlMenuID, true);
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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Quit Menu Canvas Active
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, true);
            SetChildButtonsActive(quitMenuID, true);
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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Control Menu Canvas Inactive
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, false);
            SetChildButtonsActive(controlMenuID, false);
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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Quit Menu Canvas Inactive
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
            SetChildButtonsActive(quitMenuID, false);
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
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Quit Menu Canvas Inactive
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
            SetChildButtonsActive(quitMenuID, false);
        }
	
        /* -------------------------- SETTING MENU BUTTONS ----------------------- */
        // ---------- OPEN SETTINGS BUTTON -------------
		public void _when_settings_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Settings Button");
            Bridge.SetTexturePath(buttonID, "UI-Settings-Selected");
        }
	
		public void _when_settings_button_onclick()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Settings Button");
            Bridge.SetTexturePath(buttonID, "UI-Settings");
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Settings Menu Active
            uint settingMenuID = Bridge.GetNode("Settings Menu");
            Bridge.SetCanvasActive(settingMenuID, true);
            SetChildButtonsActive(settingMenuID, true);
        }
	
        // ---------- MASTER VOLUME BUTTON -------------
		public void _when_master_button_onpressed()
        {

        }
	
        // ---------- BGM VOLUME BUTTON -------------
		public void _when_bgm_button_onpressed()
        {

        }
	
        // ---------- SFX VOLUME BUTTON -------------
		public void _when_sfx_button_onpressed()
        {

        }
	
        // ---------- SETTINGS MENU BACK BUTTON -------------
		public void _when_settings_menu_back_button_onpressed()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Settings Menu Back Button");
            Bridge.SetTexturePath(buttonID, "UI-Back-Selected");
        }
	
		public void _when_settings_menu_back_button_onclick()
        {
            // Set to button selected sprite
            uint buttonID = Bridge.GetNode("Settings Menu Back Button");
            Bridge.SetTexturePath(buttonID, "UI-Back");
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            // Set Settings Menu Canvas Inactive
            uint settingMenuID = Bridge.GetNode("Settings Menu");
            Bridge.SetCanvasActive(settingMenuID, false);
            SetChildButtonsActive(settingMenuID, false);

        }
	
		public void _when_masters_onpressed()
        {

        }
	
		public void _when_bgms_onpressed()
        {

        }
	
		public void _when_sfxs_onpressed()
        {

        }
	
		public void _when_mastersliderball_onpressed()
        {

        }
	
		public void _when_bgmsliderball_onpressed()
        {

        }
	
		public void _when_sfxsliderball_onpressed()
        {

        }
	}
}
			