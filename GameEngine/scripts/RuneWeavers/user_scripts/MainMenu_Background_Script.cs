using System;
using System.Collections.Generic;

namespace Eno
{
    public class MainMenu_Background_Script : ScriptBase
    {
        public float fadeOutDuration = 1.5f;   // seconds for the fade on Play pressed

        private float _timer = 0f;
        private const float SwapInterval = 1f;

        private static MainMenu_Background_Script? _instance;
        private bool  _fadingOut    = false;
        private float _fadeTimer    = 0f;
        private float _fadeStartVol = 100f;

        private uint  _controllerConnectID = 0;
        private float _controllerMsgTimer  = 0f;
        private bool  _wasGamepadConnected = false;

        public void Begin()
        {
            _instance   = this;
            _fadingOut  = false;
            _fadeTimer  = 0f;

            _controllerConnectID = Bridge.GetNode("ControllerConnect");
            _wasGamepadConnected = Input.isGamepadConnected();
            if (_wasGamepadConnected && _controllerConnectID != 0)
            {
                Bridge.SetTexturePath(_controllerConnectID, "UI-ControllerConnected-Text");
                Bridge.SetVisibility(_controllerConnectID, true);
                _controllerMsgTimer = 5f;
            }

            Self<Sprite2D>()?.changeState("art1");
            Bridge.SetSound(ID, "BGM_Level1a_BaseLayer_3");
            Bridge.AudioPlay(ID);
        }

        public void Update()
        {
            // Controller connect/disconnect notification
            bool gamepadNow = Input.isGamepadConnected();
            if (gamepadNow != _wasGamepadConnected)
            {
                _wasGamepadConnected = gamepadNow;
                if (_controllerConnectID != 0)
                {
                    Bridge.SetTexturePath(_controllerConnectID,
                        gamepadNow ? "UI-ControllerConnected-Text" : "UI-ControllerDisconnected-Text");
                    Bridge.SetVisibility(_controllerConnectID, true);
                }
                _controllerMsgTimer = 5f;
            }
            if (_controllerMsgTimer > 0f)
            {
                _controllerMsgTimer -= Time.deltaTime;
                if (_controllerMsgTimer <= 0f && _controllerConnectID != 0)
                    Bridge.SetVisibility(_controllerConnectID, false);
            }
            if (_fadingOut)
            {
                _fadeTimer += Bridge.GetDeltaTime();
                float t = MathF.Min(_fadeTimer / fadeOutDuration, 1f);
                Bridge.SetAudioVolume(ID, _fadeStartVol * (1f - t));
                if (t >= 1f)
                {
                    Bridge.AudioStop(ID);
                    _fadingOut = false;
                }
                return;
            }

            // Art slideshow
        }

        // Called right before ChangeScene to kill all audio immediately
        public static void StopAudio()
        {
            if (_instance == null) return;
            Bridge.AudioStop(_instance.ID);
            _instance._fadingOut = false;
        }

        // Called by Main_Menu_Script when Play is pressed
        public static void StartFadeOut()
        {
            if (_instance == null) return;
            _instance._fadeStartVol = Bridge.GetAudioVolume(_instance.ID);
            _instance._fadeTimer    = 0f;
            _instance._fadingOut    = true;
        }

        // Called when entering the art1 state
        public void art1_enter()
        {
            _timer = 0f;
        }

        // Called every frame while in the art1 state
        public void art1_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= SwapInterval)
                Self<Sprite2D>()?.changeState("art2");
        }

        // Called when exiting the art1 state
        public void art1_exit()
        {
        }

        // Called when entering the art2 state
        public void art2_enter()
        {
            _timer = 0f;
        }

        // Called every frame while in the art2 state
        public void art2_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= SwapInterval)
                Self<Sprite2D>()?.changeState("art1");
        }

        // Called when exiting the art2 state
        public void art2_exit()
        {
        }
    }
}
