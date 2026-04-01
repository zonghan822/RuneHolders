using System;
using System.Collections.Generic;

namespace Eno
{
	public class Main_Menu_Script : ScriptBase
    {
        // Node names — set these in the inspector to match your scene
        [Port] public string VideoTransitionCanvasName = "VideoTransition";
        [Port] public string CutsceneVideoNodeName     = "CutsceneVideo";

        // ------------------------------------------------------------------ //
        // Menu state
        // ------------------------------------------------------------------ //

        private enum MenuState { Main, QuitMenu, ControlsMenu, SettingsMenu, VideoTransition }
        private MenuState _menuState = MenuState.Main;

        // Main menu (W=Play N, A=Settings W, D=Controls E, S=Quit S)
        private enum MenuButton { None, Play, Settings, Controls, Quit }
        private MenuButton _selected = MenuButton.None;

        // Quit sub-menu (A=Yes left, D=No right)
        private enum QuitButton { None, Yes, No }
        private QuitButton _quitSelected = QuitButton.None;

        // Settings sub-menu (W/S cycles rows, A/D moves slider)
        private enum SettingsRow { Master = 0, BGM = 1, SFX = 2, Back = 3 }
        private SettingsRow _settingsRow = SettingsRow.Master;

        // ------------------------------------------------------------------ //
        // Slider constants — must match AudioSettings_Script
        // ------------------------------------------------------------------ //

        private const float SliderRange = 220f;
        private const float VolumeStep  = 5f;   // volume units per key press (0-100 scale)

        // Min-X anchor for each ball (back-calculated on settings open)
        private float _masterMinX;
        private float _bgmMinX;
        private float _sfxMinX;

        // Video cutscene state
        private bool _videoStarted = false;

        // ------------------------------------------------------------------ //

        public void Begin()
        {

        }

        public void Update()
        {
            if (_menuState == MenuState.Main)
            {
                if (Input.isKeyPressed(KeyCode.W)) SelectMain(MenuButton.Play);
                if (Input.isKeyPressed(KeyCode.S)) SelectMain(MenuButton.Quit);
                if (Input.isKeyPressed(KeyCode.A)) SelectMain(MenuButton.Settings);
                if (Input.isKeyPressed(KeyCode.D)) SelectMain(MenuButton.Controls);
                if (Input.isKeyPressed(KeyCode.E)) ConfirmMain();
            }
            else if (_menuState == MenuState.QuitMenu)
            {
                if (Input.isKeyPressed(KeyCode.A)) SelectQuit(QuitButton.Yes);
                if (Input.isKeyPressed(KeyCode.D)) SelectQuit(QuitButton.No);
                if (Input.isKeyPressed(KeyCode.E)) ConfirmQuit();
            }
            else if (_menuState == MenuState.ControlsMenu)
            {
                // Back is the only button — E goes back
                if (Input.isKeyPressed(KeyCode.E)) _when_control_menu_back_button_onclick();
            }
            else if (_menuState == MenuState.SettingsMenu)
            {
                if (Input.isKeyPressed(KeyCode.W)) CycleSettingsRow(-1);
                if (Input.isKeyPressed(KeyCode.S)) CycleSettingsRow(1);
                if (Input.isKeyPressed(KeyCode.A)) AdjustFocusedSlider(-VolumeStep);
                if (Input.isKeyPressed(KeyCode.D)) AdjustFocusedSlider(VolumeStep);
                if (Input.isKeyPressed(KeyCode.E) && _settingsRow == SettingsRow.Back)
                    _when_settings_menu_back_button_onclick();
            }
            else if (_menuState == MenuState.VideoTransition)
            {
                uint videoID = Bridge.GetNode(CutsceneVideoNodeName);

                // Wait until the video actually starts, then watch for it to end
                if (!_videoStarted && Bridge.IsVideoPlaying(videoID))
                    _videoStarted = true;

                // Escape skips the cutscene
                if (Input.isKeyPressed(KeyCode.Escape))
                {
                    EndCutscene();
                    return;
                }

                // Video finished naturally
                if (_videoStarted && !Bridge.IsVideoPlaying(videoID))
                    EndCutscene();
            }
        }

        // ------------------------------------------------------------------ //
        // Main menu navigation
        // ------------------------------------------------------------------ //

        private void SelectMain(MenuButton btn)
        {
            if (_selected == btn) return;
            SetMainSprite(_selected, false);
            _selected = btn;
            SetMainSprite(_selected, true);
        }

        private void SetMainSprite(MenuButton btn, bool selected)
        {
            switch (btn)
            {
                case MenuButton.Play:
                    Bridge.SetTexturePath(Bridge.GetNode("Play Button"),
                        selected ? "UI-Play-Selected" : "UI-Play");
                    break;
                case MenuButton.Settings:
                    Bridge.SetTexturePath(Bridge.GetNode("Settings Button"),
                        selected ? "UI-Settings-Selected" : "UI-Settings");
                    break;
                case MenuButton.Controls:
                    Bridge.SetTexturePath(Bridge.GetNode("Control Button"),
                        selected ? "UI-Controls-Selected" : "UI-Controls");
                    break;
                case MenuButton.Quit:
                    Bridge.SetTexturePath(Bridge.GetNode("Quit Button"),
                        selected ? "UI-QuitGame-Selected" : "UI-Quit");
                    break;
            }
        }

        private void ConfirmMain()
        {
            switch (_selected)
            {
                case MenuButton.Play:     _when_play_button_onclick();     break;
                case MenuButton.Settings: _when_settings_button_onclick(); break;
                case MenuButton.Controls: _when_control_button_onclick();  break;
                case MenuButton.Quit:     _when_quit_button_onclick();     break;
            }
        }

        // ------------------------------------------------------------------ //
        // Quit sub-menu navigation
        // ------------------------------------------------------------------ //

        private void EnterQuitMenu()
        {
            _menuState = MenuState.QuitMenu;
            SelectQuit(QuitButton.Yes);
        }

        private void SelectQuit(QuitButton btn)
        {
            if (_quitSelected == btn) return;
            SetQuitSprite(_quitSelected, false);
            _quitSelected = btn;
            SetQuitSprite(_quitSelected, true);
        }

        private void SetQuitSprite(QuitButton btn, bool selected)
        {
            switch (btn)
            {
                case QuitButton.Yes:
                    Bridge.SetTexturePath(Bridge.GetNode("Quit Menu Yes Button"),
                        selected ? "UI-Yes-Selected" : "UI-Yes");
                    break;
                case QuitButton.No:
                    Bridge.SetTexturePath(Bridge.GetNode("Quit Menu No Button"),
                        selected ? "UI-No-Selected" : "UI-No");
                    break;
            }
        }

        private void ConfirmQuit()
        {
            switch (_quitSelected)
            {
                case QuitButton.Yes: _when_quit_menu_yes_button_onclick(); break;
                case QuitButton.No:  _when_quit_menu_no_button_onclick();  break;
            }
        }

        private void ExitQuitMenu()
        {
            SetQuitSprite(_quitSelected, false);
            _quitSelected = QuitButton.None;
            _menuState = MenuState.Main;
        }

        // ------------------------------------------------------------------ //
        // Controls sub-menu navigation
        // ------------------------------------------------------------------ //

        private void EnterControlsMenu()
        {
            _menuState = MenuState.ControlsMenu;
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back-Selected");
        }

        private void ExitControlsMenu()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back");
            _menuState = MenuState.Main;
        }

        // ------------------------------------------------------------------ //
        // Settings sub-menu navigation
        // ------------------------------------------------------------------ //

        private void EnterSettingsMenu()
        {
            _menuState    = MenuState.SettingsMenu;
            _settingsRow  = SettingsRow.Master;

            // Back-calculate each slider's left-edge anchor from the saved volume
            // and the ball's current world position (AudioSettings_Script.Begin() already
            // placed each ball at the correct saved position before this opens).
            uint masterBall = Bridge.GetNode("MasterSliderBall");
            uint bgmBall    = Bridge.GetNode("BGMSliderBall");
            uint sfxBall    = Bridge.GetNode("SFXSliderBall");

            float masterVol = Audio.loadVolume("Audio Master", 0f);
            float bgmVol    = Audio.loadVolume("Audio BGM",    0f);
            float sfxVol    = Audio.loadVolume("Audio SFX",    0f);

            _masterMinX = Bridge.GetPositionX(masterBall) - (masterVol / 100f) * SliderRange;
            _bgmMinX    = Bridge.GetPositionX(bgmBall)    - (bgmVol    / 100f) * SliderRange;
            _sfxMinX    = Bridge.GetPositionX(sfxBall)    - (sfxVol    / 100f) * SliderRange;

            UpdateSettingsHighlight();
        }

        private void ExitSettingsMenu()
        {
            // Save final volumes from the ball positions
            uint masterBall = Bridge.GetNode("MasterSliderBall");
            uint bgmBall    = Bridge.GetNode("BGMSliderBall");
            uint sfxBall    = Bridge.GetNode("SFXSliderBall");

            float masterVol = BallToVolume(Bridge.GetPositionX(masterBall), _masterMinX);
            float bgmVol    = BallToVolume(Bridge.GetPositionX(bgmBall),    _bgmMinX);
            float sfxVol    = BallToVolume(Bridge.GetPositionX(sfxBall),    _sfxMinX);

            Audio.saveVolumes(masterVol, bgmVol, sfxVol);

            // Reset back button sprite if it was highlighted
            Bridge.SetTexturePath(Bridge.GetNode("Settings Menu Back Button"), "UI-Back");
            _settingsRow = SettingsRow.Master;
            _menuState   = MenuState.Main;
        }

        private void CycleSettingsRow(int dir)
        {
            int count = 4; // Master, BGM, SFX, Back
            int next  = ((int)_settingsRow + dir + count) % count;
            _settingsRow = (SettingsRow)next;
            UpdateSettingsHighlight();
        }

        // Highlights the Back button when it is focused; slider rows have no sprite change.
        private void UpdateSettingsHighlight()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Settings Menu Back Button"),
                _settingsRow == SettingsRow.Back ? "UI-Back-Selected" : "UI-Back");
        }

        private void AdjustFocusedSlider(float delta)
        {
            switch (_settingsRow)
            {
                case SettingsRow.Master:
                    AdjustBall("MasterSliderBall", _masterMinX, delta, isMaster: true);
                    break;
                case SettingsRow.BGM:
                    AdjustBall("BGMSliderBall", _bgmMinX, delta, isMaster: false, group: "BGM");
                    break;
                case SettingsRow.SFX:
                    AdjustBall("SFXSliderBall", _sfxMinX, delta, isMaster: false, group: "SFX");
                    break;
                // Back row: A/D do nothing
            }
        }

        private void AdjustBall(string ballName, float minX, float delta,
                                 bool isMaster = false, string group = "")
        {
            uint  ballID  = Bridge.GetNode(ballName);
            float current = BallToVolume(Bridge.GetPositionX(ballID), minX);
            float newVol  = MathF.Max(0f, MathF.Min(100f, current + delta));
            float newX    = minX + (newVol / 100f) * SliderRange;

            Bridge.SetPosition(ballID, newX, Bridge.GetPositionY(ballID));

            if (isMaster)
                Audio.masterVolume = newVol;
            else
                Audio.setGroupVolume(group, newVol);
        }

        private float BallToVolume(float ballX, float minX)
            => MathF.Max(0f, MathF.Min(100f, ((ballX - minX) / SliderRange) * 100f));

        // ------------------------------------------------------------------ //
        // Video cutscene
        // ------------------------------------------------------------------ //

        private void EnterCutscene()
        {
            uint canvasID = Bridge.GetNode(VideoTransitionCanvasName);
            uint videoID  = Bridge.GetNode(CutsceneVideoNodeName);

            // Stop main menu BGM so it doesn't overlap the cutscene audio
            try { Bridge.AudioStop(Bridge.GetNode("Background")); } catch (Exception) {}

            Bridge.SetCanvasActive(canvasID, true);
            Bridge.SetVisibility(videoID, true);
            Bridge.SetVideoLoop(videoID, false);
            Bridge.VideoPlay(videoID);

            _videoStarted = false;
            _menuState    = MenuState.VideoTransition;
        }

        private void EndCutscene()
        {
            uint videoID  = Bridge.GetNode(CutsceneVideoNodeName);
            uint canvasID = Bridge.GetNode(VideoTransitionCanvasName);

            Bridge.VideoStop(videoID);
            Bridge.SetVisibility(videoID, false);
            Bridge.SetCanvasActive(canvasID, false);
            Bridge.ChangeScene("Level.txt");
        }

        // ------------------------------------------------------------------ //
        // Shared helper
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

        /* -------------------------- MAIN MENU BUTTONS ----------------------- */
        // -------------- PLAY BUTTON -------------------
		public void _when_play_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Play Button"), "UI-Play-Selected");
        }

		public void _when_play_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Play Button"), "UI-Play");
            _selected = MenuButton.None;
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            EnterCutscene();
        }

        // -------------- CONTROL BUTTON -------------------
		public void _when_control_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Button"), "UI-Controls-Selected");
        }

		public void _when_control_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Button"), "UI-Controls");
            _selected = MenuButton.None;
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, true);
            SetChildButtonsActive(controlMenuID, true);
            EnterControlsMenu();
        }

        // ---------------- QUIT BUTTON -------------------
		public void _when_quit_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Button"), "UI-QuitGame-Selected");
        }

		public void _when_quit_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Button"), "UI-Quit");
            _selected = MenuButton.None;
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, true);
            SetChildButtonsActive(quitMenuID, true);
            EnterQuitMenu();
        }


        /* -------------------------- CONTROL MENU BUTTONS ----------------------- */

        // ---------- CONTROLS MENU BACK BUTTON -------------
		public void _when_control_menu_back_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back-Selected");
        }

		public void _when_control_menu_back_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back");
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint controlMenuID = Bridge.GetNode("Controls Menu");
            Bridge.SetCanvasActive(controlMenuID, false);
            SetChildButtonsActive(controlMenuID, false);
            ExitControlsMenu();
        }

        /* -------------------------- QUIT MENU BUTTONS ----------------------- */

        // ---------- QUIT MENU YES BUTTON -------------
		public void _when_quit_menu_yes_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu Yes Button"), "UI-Yes-Selected");
        }

		public void _when_quit_menu_yes_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu Yes Button"), "UI-Yes");
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
            SetChildButtonsActive(quitMenuID, false);
            ExitQuitMenu();
            Window.requestClose();
        }

        // ---------- QUIT MENU NO BUTTON -------------
		public void _when_quit_menu_no_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu No Button"), "UI-No-Selected");
        }

		public void _when_quit_menu_no_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu No Button"), "UI-No");
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint quitMenuID = Bridge.GetNode("Quit Game Menu");
            Bridge.SetCanvasActive(quitMenuID, false);
            SetChildButtonsActive(quitMenuID, false);
            ExitQuitMenu();
        }

        /* -------------------------- SETTING MENU BUTTONS ----------------------- */
        // ---------- OPEN SETTINGS BUTTON -------------
		public void _when_settings_button_onpressed()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Settings Button"), "UI-Settings-Selected");
        }

		public void _when_settings_button_onclick()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Settings Button"), "UI-Settings");
            _selected = MenuButton.None;
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint settingMenuID = Bridge.GetNode("Settings Menu");
            Bridge.SetCanvasActive(settingMenuID, true);
            SetChildButtonsActive(settingMenuID, true);
            EnterSettingsMenu();
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
            Bridge.SetTexturePath(Bridge.GetNode("Settings Menu Back Button"), "UI-Back-Selected");
        }

		public void _when_settings_menu_back_button_onclick()
        {
            try { Bridge.AudioPlay(Bridge.GetNode("sfxSoundPressE")); } catch (Exception) {}
            uint settingMenuID = Bridge.GetNode("Settings Menu");
            Bridge.SetCanvasActive(settingMenuID, false);
            SetChildButtonsActive(settingMenuID, false);
            ExitSettingsMenu();
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
