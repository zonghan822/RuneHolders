namespace Eno
{
    public class Room10_Script : ScriptBase
    {
        private bool _cleared = false;

        public void Begin() { _cleared = false; }

        public void Update()
        {
            if (_cleared) return;

            // Unlock all doors after player reads the Room10 narrative scroll (index 4)
            if (!Scroll_Dialouge_Script.DialogueShown[4]) return;

            _cleared = true;
        }
    }
}
