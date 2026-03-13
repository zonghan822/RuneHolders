namespace Eno
{
    public class Room13B_Script : ScriptBase
    {
        private static readonly string[] AllDoors =
        {
            "Door1to2",    "Door2to1",
            "Door2to3",    "Door3to2",
            "Door3to4",    "Door4to3",
            "Door4to5",    "Door5to4",
            "Door5to6",    "Door6to5",
            "Door6to7",    "Door7to6",
            "Door7to8",    "Door8to7",
            "Door8to9",    "Door9to8",
            "Door9to10",   "Door10to9",
            "Door9to11",   "Door11to9",
            "Door11to12",  "Door12to11",
            "Door12to13B", "Door13Bto12",
            "Door13Bto14B","Door14Bto13B",
            "Door14Bto15B","Door15Bto14B",
            "Door15Bto16B","Door16Bto15B",
            "Door16Bto17B","Door17Bto16B",
            "Door16Bto18B","Door18Bto16B",
            "Door18Bto19A","Door19Ato18B",
            "Door19Ato20A","Door20Ato19A",
            "Door20Ato21A","Door21Ato20A",
        };

        private bool _cleared = false;

        public void Begin() { }

        public void Update()
        {
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
