using System;

namespace Eno
{
    public class Door_Script : ScriptBase
    {
        // Fill in the scene entity names of the enemies that must die
        // before this door opens. Leave unused slots empty.
        [Port] public string Enemy1 = "";
        [Port] public string Enemy2 = "";
        [Port] public string Enemy3 = "";
        [Port] public string Enemy4 = "";
        [Port] public string Enemy5 = "";
        [Port] public string Enemy6 = "";

        // Enable this on doors that should open when the scroll dialogue finishes.
        [Port] public bool UnlockOnDialogue = false;

        private bool _opened = false;

        public void Begin() { }

        public void Update()
        {
            if (_opened) return;

            if (AllEnemiesDead())
                Open();

            if (UnlockOnDialogue && Scroll_Dialouge_Script.DialogueComplete)
                Open();
        }

        private bool AllEnemiesDead()
        {
            return IsEnemyDead(Enemy1)
                && IsEnemyDead(Enemy2)
                && IsEnemyDead(Enemy3)
                && IsEnemyDead(Enemy4)
                && IsEnemyDead(Enemy5)
                && IsEnemyDead(Enemy6);
        }

        // An enemy counts as cleared if:
        //   - the slot is empty (unused)
        //   - the node no longer exists in the scene (freed on death)
        //   - the node exists but IsDead returns true
        private bool IsEnemyDead(string name)
        {
            if (string.IsNullOrEmpty(name)) return true;
            uint id = Bridge.GetNode(name);
            if (id == 0) return true;
            return Bridge.IsDead(id);
        }

        private void Open()
        {
            _opened = true;
            // No collision-disable bridge exists, so hide the door and
            // move it far off-screen so its collider no longer blocks the player.
            Bridge.SetVisibility(ID, false);
            Bridge.SetPosition(ID, -99999f, -99999f);
        }
    }
}
