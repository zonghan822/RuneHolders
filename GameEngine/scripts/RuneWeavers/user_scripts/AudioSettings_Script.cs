#nullable enable
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
        private Sprite2D? _master;
        private Sprite2D? _bgm;
        private Sprite2D? _sfx;
        private Sprite2D? _masterFill;
        private Sprite2D? _bgmFill;
        private Sprite2D? _sfxFill;

        // Copied from statics into instance fields so Update() reads them locally
        private float _masterMinX;
        private float _bgmMinX;
        private float _sfxMinX;
        private float _masterFillY;
        private float _bgmFillY;
        private float _sfxFillY;

        private Sprite2D? _draggedBall;
        private float _dragOffsetX = 0f; // mouse-to-ball-centre offset at grab time

        public void Begin()
        {
            _master     = GetNode<Sprite2D>("MasterSliderBall");
            _bgm        = GetNode<Sprite2D>("BGMSliderBall");
            _sfx        = GetNode<Sprite2D>("SFXSliderBall");
            _masterFill = GetNode<Sprite2D>("MasterSliderFill");
            _bgmFill    = GetNode<Sprite2D>("BGMSliderFill");
            _sfxFill    = GetNode<Sprite2D>("SFXSliderFill");

            if (!s_anchored)
            {
                // Read ball positions while they are still at scene-file origin
                s_masterMinX  = _master?.position.x    ?? 0f;
                s_bgmMinX     = _bgm?.position.x       ?? 0f;
                s_sfxMinX     = _sfx?.position.x       ?? 0f;
                s_masterFillY = _masterFill?.position.y ?? 0f;
                s_bgmFillY    = _bgmFill?.position.y    ?? 0f;
                s_sfxFillY    = _sfxFill?.position.y    ?? 0f;
                s_anchored    = true;

                // Load saved volumes (0 on first ever run)
                float masterVol = Audio.loadVolume("Audio Master", 0f);
                float bgmVol    = Audio.loadVolume("Audio BGM",    0f);
                float sfxVol    = Audio.loadVolume("Audio SFX",    0f);

                Audio.masterVolume = masterVol;
                Audio.setGroupVolume("BGM", bgmVol);
                Audio.setGroupVolume("SFX", sfxVol);

                // Place balls at saved positions
                PlaceBallAtVolume(_master,     masterVol, s_masterMinX);
                PlaceBallAtVolume(_bgm,        bgmVol,    s_bgmMinX);
                PlaceBallAtVolume(_sfx,        sfxVol,    s_sfxMinX);

                // Stretch fills to match
                UpdateFill(_masterFill, s_masterMinX, VolumeToX(masterVol, s_masterMinX), s_masterFillY);
                UpdateFill(_bgmFill,    s_bgmMinX,    VolumeToX(bgmVol,    s_bgmMinX),    s_bgmFillY);
                UpdateFill(_sfxFill,    s_sfxMinX,    VolumeToX(sfxVol,    s_sfxMinX),    s_sfxFillY);
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
            Vector2 mouse = Input.mouseWorldPosition;

            if (Input.isMousePressed(0))
            {
                if (IsMouseOverBall(_master, mouse))
                {
                    _draggedBall = _master;
                    _dragOffsetX = (_master?.position.x ?? 0f) - mouse.x;
                }
                else if (IsMouseOverBall(_bgm, mouse))
                {
                    _draggedBall = _bgm;
                    _dragOffsetX = (_bgm?.position.x ?? 0f) - mouse.x;
                }
                else if (IsMouseOverBall(_sfx, mouse))
                {
                    _draggedBall = _sfx;
                    _dragOffsetX = (_sfx?.position.x ?? 0f) - mouse.x;
                }
            }

            if (Input.isMouseHeld(0) && _draggedBall != null)
            {
                float minX     = GetMinX(_draggedBall);
                // Apply grab offset so the ball doesn't snap its centre to the cursor
                float clampedX = MathF.Max(minX, MathF.Min(minX + SliderRange, mouse.x + _dragOffsetX));

                _draggedBall.position = new Vector2(clampedX, _draggedBall.position.y);
                UpdateFill(GetFill(_draggedBall), minX, clampedX, GetFillY(_draggedBall));
                ApplyVolume(_draggedBall, XToVolume(clampedX, minX));
            }

            if (Input.isMouseReleased(0) && _draggedBall != null)
            {
                // Persist all three current volumes to config.txt
                Audio.saveVolumes(
                    XToVolume(_master?.position.x ?? 0f, _masterMinX),
                    XToVolume(_bgm?.position.x    ?? 0f, _bgmMinX),
                    XToVolume(_sfx?.position.x    ?? 0f, _sfxMinX));

                _draggedBall = null;
            }
        }

        // Left edge fixed at anchorX; right edge reaches ballX.  Y never changes.
        private void UpdateFill(Sprite2D? fill, float anchorX, float ballX, float fillY)
        {
            if (fill == null) return;
            float width = MathF.Max(0f, ballX - anchorX);
            fill.position = new Vector2(anchorX + width * 0.5f, fillY);
            fill.setSize(width, fill.height);
        }

        private bool IsMouseOverBall(Sprite2D? ball, Vector2 mouse)
        {
            if (ball == null) return false;
            float dx = mouse.x - ball.position.x;
            float dy = mouse.y - ball.position.y;
            return (dx * dx + dy * dy) <= (BallRadius * BallRadius);
        }

        private void PlaceBallAtVolume(Sprite2D? ball, float volume, float minX)
        {
            if (ball == null) return;
            ball.position = new Vector2(VolumeToX(MathF.Max(0f, MathF.Min(100f, volume)), minX), ball.position.y);
        }

        private float VolumeToX(float volume, float minX)
            => minX + (volume / 100f) * SliderRange;

        private float XToVolume(float x, float minX)
            => ((x - minX) / SliderRange) * 100f;

        private float GetMinX(Sprite2D ball)
        {
            if (ball == _master) return _masterMinX;
            if (ball == _bgm)    return _bgmMinX;
            return _sfxMinX;
        }

        private Sprite2D? GetFill(Sprite2D ball)
        {
            if (ball == _master) return _masterFill;
            if (ball == _bgm)    return _bgmFill;
            return _sfxFill;
        }

        private float GetFillY(Sprite2D ball)
        {
            if (ball == _master) return _masterFillY;
            if (ball == _bgm)    return _bgmFillY;
            return _sfxFillY;
        }

        private void ApplyVolume(Sprite2D ball, float volume)
        {
            if      (ball == _master) Audio.masterVolume = volume;
            else if (ball == _bgm)    Audio.setGroupVolume("BGM", volume);
            else if (ball == _sfx)    Audio.setGroupVolume("SFX", volume);
        }
    }
}
