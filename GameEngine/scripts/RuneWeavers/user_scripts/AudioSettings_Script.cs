using System;

namespace Eno
{
    // Attach to AudioSettingsManager.
    // Finds MasterSliderBall, BGMSliderBall, SFXSliderBall by name.
    // Each ball's starting position = 0 volume. Dragging right 250 units = max volume.
    public class AudioSettings_Script : ScriptBase
    {
        private const float SliderRange = 250f;
        private const float BallRadius  = 25f;

        private uint _masterID = uint.MaxValue;
        private uint _bgmID    = uint.MaxValue;
        private uint _sfxID    = uint.MaxValue;

        private float _masterMinX;
        private float _bgmMinX;
        private float _sfxMinX;

        private uint _draggedBall = uint.MaxValue;

        public void Begin()
        {
            _masterID = Bridge.GetNode("MasterSliderBall");
            _bgmID    = Bridge.GetNode("BGMSliderBall");
            _sfxID    = Bridge.GetNode("SFXSliderBall");

            _masterMinX = Bridge.GetPositionX(_masterID);
            _bgmMinX    = Bridge.GetPositionX(_bgmID);
            _sfxMinX    = Bridge.GetPositionX(_sfxID);

            // Since the placed position represents 0 volume:
            Bridge.SetMasterVolume(0f);
            Bridge.SetGroupVolume("BGM", 0f);
            Bridge.SetGroupVolume("SFX", 0f);
        }

        public void Update()
        {
            float mx = Bridge.GetMouseWorldX();
            float my = Bridge.GetMouseWorldY();

            if (Bridge.IsMousePressed(0))
            {
                if      (IsMouseOverBall(_masterID, mx, my)) { _draggedBall = _masterID; /*Console.WriteLine("[AudioSettings] Grabbed Master slider");*/ }
                else if (IsMouseOverBall(_bgmID,    mx, my)) { _draggedBall = _bgmID;    /*Console.WriteLine("[AudioSettings] Grabbed BGM slider");*/ }
                else if (IsMouseOverBall(_sfxID,    mx, my)) { _draggedBall = _sfxID;    /*Console.WriteLine("[AudioSettings] Grabbed SFX slider");*/ }
            }

            if (Bridge.IsMouseHeld(0) && _draggedBall != uint.MaxValue)
            {
                float minX = GetMinX(_draggedBall);
                float clampedX = MathF.Max(minX, MathF.Min(minX + SliderRange, mx));
                Bridge.SetPosition(_draggedBall, clampedX, Bridge.GetPositionY(_draggedBall));

                float volume = XToVolume(clampedX, minX);
                ApplyVolume(_draggedBall, volume);
                Console.WriteLine($"[AudioSettings] Dragging - x={clampedX:F1}, volume={volume:F1}");
            }

            if (Bridge.IsMouseReleased(0) && _draggedBall != uint.MaxValue)
            {
                Console.WriteLine("[AudioSettings] Released slider");
                _draggedBall = uint.MaxValue;
            }
        }

        private bool IsMouseOverBall(uint id, float mx, float my)
        {
            if (id == uint.MaxValue) return false;
            float dx = mx - Bridge.GetPositionX(id);
            float dy = my - Bridge.GetPositionY(id);
            return (dx * dx + dy * dy) <= (BallRadius * BallRadius);
        }

        private void PlaceBallAtVolume(uint id, float volume, float minX)
        {
            if (id == uint.MaxValue) return;
            float x = VolumeToX(MathF.Max(0f, MathF.Min(100f, volume)), minX);
            Bridge.SetPosition(id, x, Bridge.GetPositionY(id));
        }

        private float VolumeToX(float volume, float minX)
            => minX + (volume / 100f) * SliderRange;

        private float XToVolume(float x, float minX)
            => ((x - minX) / SliderRange) * 100f;

        private float GetMinX(uint ballID)
        {
            if (ballID == _masterID) return _masterMinX;
            if (ballID == _bgmID)    return _bgmMinX;
            return _sfxMinX;
        }

        private void ApplyVolume(uint ballID, float volume)
        {
            if      (ballID == _masterID) Bridge.SetMasterVolume(volume);
            else if (ballID == _bgmID)    Bridge.SetGroupVolume("BGM", volume);
            else if (ballID == _sfxID)    Bridge.SetGroupVolume("SFX", volume);
        }
    }
}
