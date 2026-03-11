using System;

namespace Eno
{
    // Attach to AudioSettingsManager ONLY (not to the ball entities).
    // Finds MasterSliderBall, BGMSliderBall, SFXSliderBall by name.
    // Each ball's starting position = 0 volume. Dragging right 250 units = max volume.
    // Finds MasterSliderFill, BGMSliderFill, SFXSliderFill and stretches them horizontally.
    // Fill entities must have NO physics or collision components.
    // Volumes are saved to config.txt on release and restored on Begin().
    public class AudioSettings_Script : ScriptBase
    {
        private const float SliderRange = 250f;
        private const float BallRadius  = 25f;

        // ── Static anchors ────────────────────────────────────────────────────────
        // Captured once from the scene's original ball positions the very first time
        // Begin() runs.  Because this script may be attached to more than one entity,
        // statics guarantee only the first Begin() call reads positions (before any
        // ball has been moved by a previous instance).
        private static bool  s_anchored    = false;
        private static float s_masterMinX;
        private static float s_bgmMinX;
        private static float s_sfxMinX;
        private static float s_masterFillY;
        private static float s_bgmFillY;
        private static float s_sfxFillY;

        // ── Per-instance runtime state ────────────────────────────────────────────
        private uint _masterID     = uint.MaxValue;
        private uint _bgmID        = uint.MaxValue;
        private uint _sfxID        = uint.MaxValue;
        private uint _masterFillID = uint.MaxValue;
        private uint _bgmFillID    = uint.MaxValue;
        private uint _sfxFillID    = uint.MaxValue;

        // Copied from statics into instance fields so Update() reads them locally
        private float _masterMinX;
        private float _bgmMinX;
        private float _sfxMinX;
        private float _masterFillY;
        private float _bgmFillY;
        private float _sfxFillY;

        private uint  _draggedBall  = uint.MaxValue;
        private float _dragOffsetX  = 0f; // mouse-to-ball-centre offset at grab time

        public void Begin()
        {
            _masterID     = Bridge.GetNode("MasterSliderBall");
            _bgmID        = Bridge.GetNode("BGMSliderBall");
            _sfxID        = Bridge.GetNode("SFXSliderBall");
            _masterFillID = Bridge.GetNode("MasterSliderFill");
            _bgmFillID    = Bridge.GetNode("BGMSliderFill");
            _sfxFillID    = Bridge.GetNode("SFXSliderFill");

            if (!s_anchored)
            {
                // Read ball positions while they are still at scene-file origin
                s_masterMinX  = Bridge.GetPositionX(_masterID);
                s_bgmMinX     = Bridge.GetPositionX(_bgmID);
                s_sfxMinX     = Bridge.GetPositionX(_sfxID);
                s_masterFillY = Bridge.GetPositionY(_masterFillID);
                s_bgmFillY    = Bridge.GetPositionY(_bgmFillID);
                s_sfxFillY    = Bridge.GetPositionY(_sfxFillID);
                s_anchored    = true;

                // Load saved volumes (0 on first ever run)
                float masterVol = Bridge.LoadAudioVolume("Audio Master", 0f);
                float bgmVol    = Bridge.LoadAudioVolume("Audio BGM",    0f);
                float sfxVol    = Bridge.LoadAudioVolume("Audio SFX",    0f);

                Bridge.SetMasterVolume(masterVol);
                Bridge.SetGroupVolume("BGM", bgmVol);
                Bridge.SetGroupVolume("SFX", sfxVol);

                // Place balls at saved positions
                PlaceBallAtVolume(_masterID, masterVol, s_masterMinX);
                PlaceBallAtVolume(_bgmID,    bgmVol,    s_bgmMinX);
                PlaceBallAtVolume(_sfxID,    sfxVol,    s_sfxMinX);

                // Stretch fills to match
                UpdateFill(_masterFillID, s_masterMinX, VolumeToX(masterVol, s_masterMinX), s_masterFillY);
                UpdateFill(_bgmFillID,    s_bgmMinX,    VolumeToX(bgmVol,    s_bgmMinX),    s_bgmFillY);
                UpdateFill(_sfxFillID,    s_sfxMinX,    VolumeToX(sfxVol,    s_sfxMinX),    s_sfxFillY);
            }

            // Every instance copies from statics so Update() has correct local values
            _masterMinX  = s_masterMinX;
            _bgmMinX     = s_bgmMinX;
            _sfxMinX     = s_sfxMinX;
            _masterFillY = s_masterFillY;
            _bgmFillY    = s_bgmFillY;
            _sfxFillY    = s_sfxFillY;
        }

        public void Update()
        {
            float mx = Bridge.GetMouseWorldX();
            float my = Bridge.GetMouseWorldY();

            if (Bridge.IsMousePressed(0))
            {
                if (IsMouseOverBall(_masterID, mx, my))
                {
                    _draggedBall = _masterID;
                    _dragOffsetX = Bridge.GetPositionX(_masterID) - mx;
                }
                else if (IsMouseOverBall(_bgmID, mx, my))
                {
                    _draggedBall = _bgmID;
                    _dragOffsetX = Bridge.GetPositionX(_bgmID) - mx;
                }
                else if (IsMouseOverBall(_sfxID, mx, my))
                {
                    _draggedBall = _sfxID;
                    _dragOffsetX = Bridge.GetPositionX(_sfxID) - mx;
                }
            }

            if (Bridge.IsMouseHeld(0) && _draggedBall != uint.MaxValue)
            {
                float minX     = GetMinX(_draggedBall);
                // Apply grab offset so the ball doesn't snap its centre to the cursor
                float clampedX = MathF.Max(minX, MathF.Min(minX + SliderRange, mx + _dragOffsetX));

                Bridge.SetPosition(_draggedBall, clampedX, Bridge.GetPositionY(_draggedBall));
                UpdateFill(GetFillID(_draggedBall), minX, clampedX, GetFillY(_draggedBall));
                ApplyVolume(_draggedBall, XToVolume(clampedX, minX));
            }

            if (Bridge.IsMouseReleased(0) && _draggedBall != uint.MaxValue)
            {
                // Persist all three current volumes to config.txt
                Bridge.SaveAudioVolumes(
                    XToVolume(Bridge.GetPositionX(_masterID), _masterMinX),
                    XToVolume(Bridge.GetPositionX(_bgmID),    _bgmMinX),
                    XToVolume(Bridge.GetPositionX(_sfxID),    _sfxMinX));

                _draggedBall = uint.MaxValue;
            }
        }

        // Left edge fixed at anchorX; right edge reaches ballX.  Y never changes.
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

        private void PlaceBallAtVolume(uint id, float volume, float minX)
        {
            if (id == uint.MaxValue) return;
            Bridge.SetPosition(id, VolumeToX(MathF.Max(0f, MathF.Min(100f, volume)), minX),
                                   Bridge.GetPositionY(id));
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
