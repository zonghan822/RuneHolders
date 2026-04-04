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

        public void Begin()
        {
            _instance   = this;
            _fadingOut  = false;
            _fadeTimer  = 0f;

            Self<Sprite2D>()?.changeState("art1");
            Bridge.SetSound(ID, "BGM_Level1a_BaseLayer_3");
            Bridge.AudioPlay(ID);
        }

        public void Update()
        {
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

        // Called by Main_Menu_Script when Play is pressed
        public static void StartFadeOut()
        {
            if (_instance == null) return;
            _instance._fadeStartVol = Bridge.GetAudioVolume(_instance.ID);
            _instance._fadeTimer    = 0f;
            _instance._fadingOut    = true;
        }

        // Called by Main_Menu_Script just before ChangeScene to guarantee silence
        public static void StopNow()
        {
            if (_instance == null) return;
            Bridge.AudioStop(_instance.ID);
            _instance._fadingOut = false;
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
