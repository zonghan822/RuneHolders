using System;

namespace Eno
{
    // Attach to one entity per scene.
    // On scene load, instantly sets BGM volume to 0 then ramps it up to the
    // volume last saved in config (falling back to FallbackVolume if no config exists).
    public class BGMFadeIn_Script : ScriptBase
    {
        // How long the fade-in takes (seconds)
        public float FadeInDuration = 1.0f;

        // Volume used when no saved config value is found (0-100)
        public float FallbackVolume = 100f;

        private float _targetVol = 100f;
        private float _timer     = 0f;
        private bool  _fading    = false;

        public void Begin()
        {
            float saved = Bridge.LoadAudioVolume("Audio BGM", -1f);
            _targetVol  = (saved >= 0f) ? saved : FallbackVolume;

            Bridge.SetGroupVolume("BGM", 0f);
            _timer  = 0f;
            _fading = true;
        }

        public void Update()
        {
            if (!_fading) return;

            _timer += Bridge.GetDeltaTime();
            float t = Math.Min(_timer / FadeInDuration, 1f);

            Bridge.SetGroupVolume("BGM", _targetVol * t);

            if (t >= 1f)
                _fading = false;
        }
    }
}
