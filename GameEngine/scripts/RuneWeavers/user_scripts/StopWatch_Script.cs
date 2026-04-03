using System;
using System.Collections.Generic;

namespace Eno
{
    /*============================================================================================
    * StopWatch_Script
    * ---------------
    * Attach to the StopWatch Sprite2D node that lives inside a Canvas parent.
    *
    * State machine states expected on this node: FadeIn | Idle | FadeOut
    *   FadeIn  — plays the appear animation; transitions to Idle after fadeInTime  real seconds.
    *   FadeOut — plays the disappear animation; hides parent Canvas after fadeOutTime real seconds.
    *
    * Timers use unscaled delta time so they always run at real-world speed even when the
    * game world is slowed via Time.timeScale.
    *
    * Driven externally by RuneSystem which calls changeState("FadeIn") / changeState("FadeOut")
    * directly on the StopWatch Sprite2D node.
    *============================================================================================*/
    public class StopWatch_Script : ScriptBase
    {
        // ── Tuneable — set in the editor ──────────────────────────────────────
        public float fadeInTime  = 0.3f;
        public float fadeOutTime = 0.3f;

        // ── Per-state timer ───────────────────────────────────────────────────
        private float _timer = 0f;

        private Sprite2D?  _self;
        private AudioNode? _audio;

        public void Begin()
        {
            _self = Self<Sprite2D>();
            if (_self == null) return;
            _self.changeState("Idle");
            _self.visible = false;

            _audio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public void Update() { }

        // ── State: FadeIn ─────────────────────────────────────────────────────
        public void FadeIn_enter()
        {
            _timer = 0f;
            if (_self == null) return;
            if (_audio != null)
            {
                _audio.sound = "TimeStop_v1";
                _audio.play();
            }
            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            // Animation system uses scaled dt, so multiply by timeScale to keep real duration correct
            if (frames > 0) _self.setAnimationFrameDuration(fadeInTime * Time.timeScale / frames);
        }

        public void FadeIn_update()
        {
            _timer += Time.unscaledDeltaTime;
            if (_timer >= fadeInTime)
                _self.changeState("Idle");
        }

        public void FadeIn_exit() {}

        // ── State: Idle ───────────────────────────────────────────────────────
        public void Idle_enter() {}
        public void Idle_update() 
        {
            if (_self == null) return;
            _self.setAnimationLooping(true);
        }
        public void Idle_exit() {}

        // ── State: FadeOut ────────────────────────────────────────────────────
        public void FadeOut_enter()
        {
            _timer = 0f;
            if (_self == null) return;
            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            // timeScale is 1.0 during FadeOut (restored by EndSlowTime before this state), so this is just fadeOutTime / frames
            if (frames > 0) _self.setAnimationFrameDuration(fadeOutTime * Time.timeScale / frames);
        }

        public void FadeOut_update()
        {
            _timer += Time.unscaledDeltaTime;
            if (_timer >= fadeOutTime)
            {
                // Hide this node once the fade-out animation finishes
                if (_self == null) return;
                _self.changeState("Idle");
                _self.visible = false;
            }
        }

        public void FadeOut_exit() {}
    }
}
