namespace Eno
{
    public class Room10_Script : ScriptBase
    {
        private bool _cleared = false;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            _cleared = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public void Update()
        {
            if (_cleared) return;

            // Unlock all doors after player reads the Room10 narrative scroll
            if (!Room10_Scroll_Script.Shown) return;

            _cleared = true;
            UnlockDoor("Door10to9");
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
            {
                Bridge.ChangeState(id, "Unlocking");
                if (_doorAudio != null) { _doorAudio.sound = "Door_Open_02"; _doorAudio.play(); }
            }
        }
    }
}
