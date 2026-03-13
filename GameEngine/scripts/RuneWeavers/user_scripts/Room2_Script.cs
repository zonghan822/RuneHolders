namespace Eno
{
    public class Room2_Script : ScriptBase
    {
        private static readonly string[] AllDoors =
        {
            "Door1to2",  "Door2to1",
            "Door2to3",  "Door3to2",
            "Door3to4",  "Door4to3",
            "Door4to5",  "Door5to4",
            "Door5to6",  "Door6to5",
            "Door6to7",  "Door7to6",
            "Door7to8",  "Door8to7",
            "Door8to9",  "Door9to8",
            "Door9to10", "Door10to9",
            "Door9to11", "Door11to9",
        };

        private bool _cleared       = false;
        private bool _videoStarted  = false;

        private Canvas?    _videoCanvas = null;
        private VideoNode? _videoHolder = null;

        public void Begin()
        {
            try { _videoCanvas = GetNode<Canvas>("VideoCanvas");    } catch { }
            try { _videoHolder = GetNode<VideoNode>("VideoHolder"); } catch { }

            if (_videoCanvas != null) _videoCanvas.active = true;
        }

        public void Update()
        {
            if (!_videoStarted && _videoHolder != null)
            {
                _videoStarted = true;
                _videoHolder.play();
            }

            if (_cleared) return;

            // TODO: replace with real clear condition
            if (!Input.isKeyPressed(KeyCode.Space)) return;

            _cleared = true;
            foreach (string name in AllDoors)
            {
                uint id = Bridge.GetNode(name);
                if (id != 0 && id != uint.MaxValue)
                    Bridge.ChangeState(id, "Unlocking");
            }
        }
    }
}
