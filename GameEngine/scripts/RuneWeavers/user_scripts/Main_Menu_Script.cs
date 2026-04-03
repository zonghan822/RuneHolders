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

        // Video cutscene state — timer guards against false "ended" on frame 1
        private float _videoTimer  = 0f;

        // Left stick edge detection
        private bool _stickLeft  = false;
        private bool _stickRight = false;
        private bool _stickUp    = false;
        private bool _stickDown  = false;

        // Hover / click audio
        private AudioNode? _hoverAudio          = null;
        private AudioNode? _clickAudio          = null;
        private string     _hoveredButtonName    = "";
        private string     _prevHoveredButtonName = "";

        // ------------------------------------------------------------------ //

        private static Main_Menu_Script? _instance;

        public void Begin()
        {
            _instance   = this;
            _hoverAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            _clickAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        private void PlayClickSound()
        {
            if (_clickAudio == null) return;
            _clickAudio.sound = "UI_Select_Small 1";
            _clickAudio.play();
        }

        // Called by AudioSettings_Script when the player clicks a slider ball directly,
        // so the settings highlight (e.g. Back button) updates to match.
        public static void NotifyBallGrabbed(string ballName)
        {
            if (_instance == null || _instance._menuState != MenuState.SettingsMenu) return;
            _instance._settingsRow = ballName == "MasterSliderBall" ? SettingsRow.Master
                                   : ballName == "BGMSliderBall"    ? SettingsRow.BGM
                                   :                                  SettingsRow.SFX;
            _instance.UpdateSettingsHighlight();
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            // Left stick edge detection — only when using controller
            bool stickLeftEdge = false, stickRightEdge = false, stickUpEdge = false, stickDownEdge = false;
            if (isController)
            {
                const float STICK_THRESHOLD = 0.5f;
                Vector2 stick = Input.getLeftStick(0.2f);
                bool stickLeftNow  = stick.x < -STICK_THRESHOLD;
                bool stickRightNow = stick.x >  STICK_THRESHOLD;
                bool stickUpNow    = stick.y < -STICK_THRESHOLD;
                bool stickDownNow  = stick.y >  STICK_THRESHOLD;
                stickLeftEdge  = stickLeftNow  && !_stickLeft;
                stickRightEdge = stickRightNow && !_stickRight;
                stickUpEdge    = stickUpNow    && !_stickUp;
                stickDownEdge  = stickDownNow  && !_stickDown;
                _stickLeft  = stickLeftNow;
                _stickRight = stickRightNow;
                _stickUp    = stickUpNow;
                _stickDown  = stickDownNow;
            }
            else
            {
                _stickLeft = _stickRight = _stickUp = _stickDown = false;
            }

            bool up    = Input.isKeyPressed(KeyCode.W) || (isController && (Input.isGamepadButtonPressed(GamepadButton.DPadUp)    || stickUpEdge));
            bool down  = Input.isKeyPressed(KeyCode.S) || (isController && (Input.isGamepadButtonPressed(GamepadButton.DPadDown)  || stickDownEdge));
            bool left  = Input.isKeyPressed(KeyCode.A) || (isController && (Input.isGamepadButtonPressed(GamepadButton.DPadLeft)  || stickLeftEdge));
            bool right = Input.isKeyPressed(KeyCode.D) || (isController && (Input.isGamepadButtonPressed(GamepadButton.DPadRight) || stickRightEdge));
            bool confirm = Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.A));
            bool back    = isController && Input.isGamepadButtonPressed(GamepadButton.B);

            if (_menuState == MenuState.Main)
            {
                if (up)    SelectMain(MenuButton.Play);
                if (down)  SelectMain(MenuButton.Quit);
                if (left)  SelectMain(MenuButton.Settings);
                if (right) SelectMain(MenuButton.Controls);
                if (confirm) ConfirmMain();
            }
            else if (_menuState == MenuState.QuitMenu)
            {
                if (left)  SelectQuit(QuitButton.Yes);
                if (right) SelectQuit(QuitButton.No);
                if (confirm) ConfirmQuit();
                if (back)    { _when_quit_menu_no_button_onclick(); }
            }
            else if (_menuState == MenuState.ControlsMenu)
            {
                if (confirm || back) _when_control_menu_back_button_onclick();
            }
            else if (_menuState == MenuState.SettingsMenu)
            {
                if (up)    CycleSettingsRow(-1);
                if (down)  CycleSettingsRow(1);
                if (left)  AdjustFocusedSlider(-VolumeStep);
                if (right) AdjustFocusedSlider(VolumeStep);
                if (confirm && _settingsRow == SettingsRow.Back)
                    _when_settings_menu_back_button_onclick();
                if (back) _when_settings_menu_back_button_onclick();
            }
            else if (_menuState == MenuState.VideoTransition)
            {
                uint videoID = Bridge.GetNode(CutsceneVideoNodeName);

                _videoTimer += Time.deltaTime;

                // Escape skips the cutscene
                if (Input.isKeyPressed(KeyCode.Escape) || (isController && Input.isGamepadButtonPressed(GamepadButton.B)))
                {
                    EndCutscene();
                    return;
                }

                if (_videoTimer > 0.3f)
                {
                    float vDur      = Bridge.GetVideoDuration(videoID);
                    float vTime     = Bridge.GetVideoTime(videoID);
                    bool  isPlaying = Bridge.IsVideoPlaying(videoID);

                    // Debug: print state every ~60 frames
                    if ((int)(_videoTimer * 60f) % 60 == 0)
                        Console.WriteLine($"[Video] t={_videoTimer:F1}s vTime={vTime:F1}s vDur={vDur:F1}s playing={isPlaying}");

                    // Primary: video player state is no longer PLAYING
                    bool stateEnded = !isPlaying;
                    // Fallback: elapsed video time has reached the container duration
                    bool timeEnded  = vDur > 1f && vTime >= vDur - 0.5f;

                    if (stateEnded || timeEnded)
                    {
                        Console.WriteLine($"[Video] Ending cutscene: stateEnded={stateEnded} timeEnded={timeEnded}");
                        EndCutscene();
                    }
                }
            }

            // Snapshot hover state: handlers set _hoveredButtonName after Update runs,
            // so save it now as the "last frame" reference for next frame's edge detection.
            _prevHoveredButtonName = _hoveredButtonName;
            _hoveredButtonName     = "";
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
            SetMainButtonsActive(false);
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
            SetMainButtonsActive(true);
        }

        // ------------------------------------------------------------------ //
        // Controls sub-menu navigation
        // ------------------------------------------------------------------ //

        private void EnterControlsMenu()
        {
            SetMainButtonsActive(false);
            _menuState = MenuState.ControlsMenu;
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back-Selected");
        }

        private void ExitControlsMenu()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back");
            _menuState = MenuState.Main;
            SetMainButtonsActive(true);
        }

        // ------------------------------------------------------------------ //
        // Settings sub-menu navigation
        // ------------------------------------------------------------------ //

        private void EnterSettingsMenu()
        {
            SetMainButtonsActive(false);
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

            // Reset back button sprite and all slider balls
            Bridge.SetTexturePath(Bridge.GetNode("Settings Menu Back Button"), "UI-Back");
            SetMainButtonsActive(true);
            Bridge.ChangeState(Bridge.GetNode("MasterSliderBall"), "Default");
            Bridge.ChangeState(Bridge.GetNode("BGMSliderBall"),    "Default");
            Bridge.ChangeState(Bridge.GetNode("SFXSliderBall"),    "Default");
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

        // Highlights the Back button and slider balls based on the focused row.
        private void UpdateSettingsHighlight()
        {
            Bridge.SetTexturePath(Bridge.GetNode("Settings Menu Back Button"),
                _settingsRow == SettingsRow.Back ? "UI-Back-Selected" : "UI-Back");
            Bridge.ChangeState(Bridge.GetNode("MasterSliderBall"), _settingsRow == SettingsRow.Master ? "selected" : "Default");
            Bridge.ChangeState(Bridge.GetNode("BGMSliderBall"),    _settingsRow == SettingsRow.BGM    ? "selected" : "Default");
            Bridge.ChangeState(Bridge.GetNode("SFXSliderBall"),    _settingsRow == SettingsRow.SFX    ? "selected" : "Default");
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
            SetMainButtonsActive(false);
            uint canvasID = Bridge.GetNode(VideoTransitionCanvasName);
            uint videoID  = Bridge.GetNode(CutsceneVideoNodeName);

            // Fade out main menu BGM into the cutscene
            MainMenu_Background_Script.StartFadeOut();

            Bridge.SetCanvasActive(canvasID, true);
            Bridge.SetVisibility(videoID, true);
            Bridge.SetVideoLoop(videoID, false);
            Bridge.VideoPlay(videoID);

            _videoTimer = 0f;
            _menuState  = MenuState.VideoTransition;
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

        private void SetMainButtonsActive(bool active)
        {
            string[] names = { "Play Button", "Settings Button", "Control Button", "Quit Button" };
            foreach (string name in names)
            {
                uint id = Bridge.GetNode(name);
                if (id != 0 && id != uint.MaxValue)
                    Bridge.SetButtonActive(id, active);
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

        /* -------------------------- HOVER HANDLERS ----------------------------- */

        private void OnButtonHover(string name)
        {
            _hoveredButtonName = name;
            if (_prevHoveredButtonName == name) return;   // still on same button — skip

            // Change sprite to selected state (mirrors WASD selection)
            if (_menuState == MenuState.Main)
            {
                MenuButton btn = name switch
                {
                    "Play"     => MenuButton.Play,
                    "Controls" => MenuButton.Controls,
                    "Quit"     => MenuButton.Quit,
                    "Settings" => MenuButton.Settings,
                    _          => MenuButton.None
                };
                if (btn != MenuButton.None)
                    SelectMain(btn);
            }

            if (_hoverAudio == null) return;
            _hoverAudio.sound = "UI_Hover 6";
            _hoverAudio.play();
        }

        public void _when_play_button_onhover()     => OnButtonHover("Play");
        public void _when_control_button_onhover()  => OnButtonHover("Controls");
        public void _when_quit_button_onhover()     => OnButtonHover("Quit");
        public void _when_settings_button_onhover() => OnButtonHover("Settings");

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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Control Button"), "UI-Controls");
            _selected = MenuButton.None;
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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Quit Button"), "UI-Quit");
            _selected = MenuButton.None;
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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Control Menu Back Button"), "UI-Back");
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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu Yes Button"), "UI-Yes");
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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Quit Menu No Button"), "UI-No");
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
            PlayClickSound();
            Bridge.SetTexturePath(Bridge.GetNode("Settings Button"), "UI-Settings");
            _selected = MenuButton.None;
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
            PlayClickSound();
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
	
		public void _when_quit_menu_yes_button_onhover()
        {
            _hoveredButtonName = "QuitYes";
            if (_prevHoveredButtonName == "QuitYes") return;
            if (_menuState == MenuState.QuitMenu) SelectQuit(QuitButton.Yes);
            if (_hoverAudio == null) return;
            _hoverAudio.sound = "UI_Hover 6";
            _hoverAudio.play();
        }

		public void _when_quit_menu_no_button_onhover()
        {
            _hoveredButtonName = "QuitNo";
            if (_prevHoveredButtonName == "QuitNo") return;
            if (_menuState == MenuState.QuitMenu) SelectQuit(QuitButton.No);
            if (_hoverAudio == null) return;
            _hoverAudio.sound = "UI_Hover 6";
            _hoverAudio.play();
        }

		public void _when_settings_menu_back_button_onhover()
        {
            _hoveredButtonName = "SettingsBack";
            if (_prevHoveredButtonName == "SettingsBack") return;
            if (_menuState == MenuState.SettingsMenu)
            {
                _settingsRow = SettingsRow.Back;
                UpdateSettingsHighlight();
            }
            if (_hoverAudio == null) return;
            _hoverAudio.sound = "UI_Hover 6";
            _hoverAudio.play();
        }
	}
}
