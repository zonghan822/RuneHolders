namespace Eno
{
    public class Room10_Script : ScriptBase
    {
        private bool _cleared = false;

        public void Begin() { _cleared = false; }

        public void Update()
        {
            if (_cleared) return;

            // Unlock all doors after player reads the Room10 narrative scroll
            if (!Room10_Scroll_Script.Shown) return;

            _cleared = true;
        }
    }
}
