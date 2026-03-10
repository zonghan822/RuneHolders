using System;

namespace Eno
{
    // Attach to AudioSettingsManager.
    // Finds MasterSliderBall, BGMSliderBall, SFXSliderBall by name.
    // Drag a ball left/right within [SliderMinX, SliderMaxX] to change its volume (0-100).
    public class AudioSettings_Script : ScriptBase
    {
        private const float SliderMinX = 710f;
        private const float SliderMaxX = 1210f;
        private const float BallRadius  = 25f;

        private uint _masterID = uint.MaxValue;
        private uint _bgmID    = uint.MaxValue;
        private uint _sfxID    = uint.MaxValue;

        private uint _draggedBall = uint.MaxValue;

        public void Begin()
        {
            _masterID = Bridge.GetNode("MasterSliderBall");
            _bgmID    = Bridge.GetNode("BGMSliderBall");
            _sfxID    = Bridge.GetNode("SFXSliderBall");

            Console.WriteLine($"[AudioSettings] Begin — Master={_masterID}, BGM={_bgmID}, SFX={_sfxID}");

            // Sync ball positions to current engine volumes
            PlaceBallAtVolume(_masterID, Bridge.GetMasterVolume());
            PlaceBallAtVolume(_bgmID,    Bridge.GetGroupVolume("BGM"));
            PlaceBallAtVolume(_sfxID,    Bridge.GetGroupVolume("SFX"));
        }

        public void Update()
        {
            float mx = Bridge.GetMouseWorldX();
            float my = Bridge.GetMouseWorldY();

            if (Bridge.IsMousePressed(0))
            {
                if      (IsMouseOverBall(_masterID, mx, my)) { _draggedBall = _masterID; Console.WriteLine("[AudioSettings] Grabbed Master slider"); }
                else if (IsMouseOverBall(_bgmID,    mx, my)) { _draggedBall = _bgmID;    Console.WriteLine("[AudioSettings] Grabbed BGM slider"); }
                else if (IsMouseOverBall(_sfxID,    mx, my)) { _draggedBall = _sfxID;    Console.WriteLine("[AudioSettings] Grabbed SFX slider"); }
            }

            if (Bridge.IsMouseHeld(0) && _draggedBall != uint.MaxValue)
            {
                float clampedX = MathF.Max(SliderMinX, MathF.Min(SliderMaxX, mx));
                Bridge.SetPosition(_draggedBall, clampedX, Bridge.GetPositionY(_draggedBall));

                float volume = XToVolume(clampedX);
                ApplyVolume(_draggedBall, volume);
                Console.WriteLine($"[AudioSettings] Dragging — x={clampedX:F1}, volume={volume:F1}");
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

        private void PlaceBallAtVolume(uint id, float volume)
        {
            if (id == uint.MaxValue) return;
            float x = VolumeToX(MathF.Max(0f, MathF.Min(100f, volume)));
            Bridge.SetPosition(id, x, Bridge.GetPositionY(id));
        }

        private float VolumeToX(float volume)
            => SliderMinX + (volume / 100f) * (SliderMaxX - SliderMinX);

        private float XToVolume(float x)
            => ((x - SliderMinX) / (SliderMaxX - SliderMinX)) * 100f;

        private void ApplyVolume(uint ballID, float volume)
        {
            if      (ballID == _masterID) Bridge.SetMasterVolume(volume);
            else if (ballID == _bgmID)    Bridge.SetGroupVolume("BGM", volume);
            else if (ballID == _sfxID)    Bridge.SetGroupVolume("SFX", volume);
        }
    }
}
