using System;

namespace Eno
{
    // Attach to AudioSettingsManager.
    // Finds MasterSliderBall, BGMSliderBall, SFXSliderBall by name.
    // Each ball's starting position = 0 volume. Dragging right 250 units = max volume.
    // Finds MasterSliderFill, BGMSliderFill, SFXSliderFill – stretches each fill
    // horizontally only: left edge is fixed at the ball's 0-vol X, right edge follows the ball.
    // Fill entities must have NO physics or collision components.
    public class AudioSettings_Script : ScriptBase
    {
        private const float SliderRange = 250f;
        private const float BallRadius  = 25f;

        private uint _masterID = uint.MaxValue;
        private uint _bgmID    = uint.MaxValue;
        private uint _sfxID    = uint.MaxValue;

        private uint _masterFillID = uint.MaxValue;
        private uint _bgmFillID    = uint.MaxValue;
        private uint _sfxFillID    = uint.MaxValue;

        private float _masterMinX;
        private float _bgmMinX;
        private float _sfxMinX;

        // Fill Y values captured once at Begin() – never written again
        private float _masterFillY;
        private float _bgmFillY;
        private float _sfxFillY;

        private uint _draggedBall = uint.MaxValue;

        public void Begin()
        {
            _masterID = Bridge.GetNode("MasterSliderBall");
            _bgmID    = Bridge.GetNode("BGMSliderBall");
            _sfxID    = Bridge.GetNode("SFXSliderBall");

            _masterFillID = Bridge.GetNode("MasterSliderFill");
            _bgmFillID    = Bridge.GetNode("BGMSliderFill");
            _sfxFillID    = Bridge.GetNode("SFXSliderFill");

            // Ball 0-vol anchors
            _masterMinX = Bridge.GetPositionX(_masterID);
            _bgmMinX    = Bridge.GetPositionX(_bgmID);
            _sfxMinX    = Bridge.GetPositionX(_sfxID);

            // Capture fill Y once – never touched again
            _masterFillY = Bridge.GetPositionY(_masterFillID);
            _bgmFillY    = Bridge.GetPositionY(_bgmFillID);
            _sfxFillY    = Bridge.GetPositionY(_sfxFillID);

            Bridge.SetMasterVolume(0f);
            Bridge.SetGroupVolume("BGM", 0f);
            Bridge.SetGroupVolume("SFX", 0f);

            // Initialise fills to zero width
            UpdateFill(_masterFillID, _masterMinX, _masterMinX, _masterFillY);
            UpdateFill(_bgmFillID,    _bgmMinX,    _bgmMinX,    _bgmFillY);
            UpdateFill(_sfxFillID,    _sfxMinX,    _sfxMinX,    _sfxFillY);
        }

        public void Update()
        {
            float mx = Bridge.GetMouseWorldX();
            float my = Bridge.GetMouseWorldY();

            if (Bridge.IsMousePressed(0))
            {
                if      (IsMouseOverBall(_masterID, mx, my)) _draggedBall = _masterID;
                else if (IsMouseOverBall(_bgmID,    mx, my)) _draggedBall = _bgmID;
                else if (IsMouseOverBall(_sfxID,    mx, my)) _draggedBall = _sfxID;
            }

            if (Bridge.IsMouseHeld(0) && _draggedBall != uint.MaxValue)
            {
                float minX     = GetMinX(_draggedBall);
                float clampedX = MathF.Max(minX, MathF.Min(minX + SliderRange, mx));

                // Move ball horizontally only
                Bridge.SetPosition(_draggedBall, clampedX, Bridge.GetPositionY(_draggedBall));

                // Stretch fill from left anchor to ball – Y never changes
                UpdateFill(GetFillID(_draggedBall), minX, clampedX, GetFillY(_draggedBall));

                ApplyVolume(_draggedBall, XToVolume(clampedX, minX));
            }

            if (Bridge.IsMouseReleased(0) && _draggedBall != uint.MaxValue)
                _draggedBall = uint.MaxValue;
        }

        // Left edge stays fixed at anchorX; right edge reaches ballX.
        // Only X and width change – Y is the value captured at Begin().
        private void UpdateFill(uint fillID, float anchorX, float ballX, float fillY)
        {
            if (fillID == uint.MaxValue) return;
            float width = MathF.Max(0f, ballX - anchorX);
            Bridge.SetPosition(fillID, anchorX + width * 0.5f, fillY);
            Bridge.SetRenderSize(fillID, width, Bridge.GetRenderHeight(fillID));
        }

        private bool IsMouseOverBall(uint id, float mx, float my)
        {
            if (id == uint.MaxValue) return false;
            float dx = mx - Bridge.GetPositionX(id);
            float dy = my - Bridge.GetPositionY(id);
            return (dx * dx + dy * dy) <= (BallRadius * BallRadius);
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

        private uint GetFillID(uint ballID)
        {
            if (ballID == _masterID) return _masterFillID;
            if (ballID == _bgmID)    return _bgmFillID;
            return _sfxFillID;
        }

        private float GetFillY(uint ballID)
        {
            if (ballID == _masterID) return _masterFillY;
            if (ballID == _bgmID)    return _bgmFillY;
            return _sfxFillY;
        }

        private void ApplyVolume(uint ballID, float volume)
        {
            if      (ballID == _masterID) Bridge.SetMasterVolume(volume);
            else if (ballID == _bgmID)    Bridge.SetGroupVolume("BGM", volume);
            else if (ballID == _sfxID)    Bridge.SetGroupVolume("SFX", volume);
        }
    }
}
