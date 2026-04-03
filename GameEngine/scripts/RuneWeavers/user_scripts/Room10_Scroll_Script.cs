using System;

namespace Eno
{
    public class Room10_Scroll_Script : ScriptBase
    {
        private Canvas?   _canvas;
        private Sprite2D? _narrative;
        public static bool Shown = false;
        private bool _open  = false;
        private bool _skipE = false;
        private AudioNode? _scrollAudio = null;

        public void Begin()
        {
            try { _canvas    = GetNode<Canvas>("Dialogue Canvas"); } catch { }
            try { _narrative = GetNode<Sprite2D>("Narrative");      } catch { }
            if (_canvas != null) _canvas.active = false;
            Shown  = false;
            _open  = false;
            _skipE = false;
            _scrollAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public void Update()
        {
            uint playerID = Bridge.GetNode("Player");
            if (playerID == 0 || playerID == uint.MaxValue) return;

            bool isController = Input.inputMode == InputMode.Controller;
            if (_open && (Input.isKeyPressed(KeyCode.Escape) || (isController && Input.isGamepadButtonPressed(GamepadButton.B))))
            {
                Close();
                return;
            }

            bool ePressed = (Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.A))) && !_skipE;
            _skipE = false;

            if (!_open && !Scroll_Dialouge_Script.DialogueActive
                && Bridge.IsColliding(ID, playerID) && ePressed)
            {
                Open();
            }
            else if (_open && ePressed)
            {
                Close();
            }
        }

        private void Open()
        {
            bool firstOpen = !Shown;
            Shown = _open = true;
            _skipE = true;
            Scroll_Dialouge_Script.DialogueActive = true;
            if (_narrative != null) { _narrative.texture = "UI-Narrative-3"; _narrative.visible = true; }
            if (_canvas    != null) _canvas.active = true;
            if (_scrollAudio != null) { _scrollAudio.sound = "Spellbook_Open_01"; _scrollAudio.play(); }
            Bridge.SetButtonActive(Bridge.GetNode("NarrativeBackButton"), true);
            if (firstOpen) UnlockDoor("Door10to9");
        }

        private void UnlockDoor(string name)
        {
            uint id = Bridge.GetNode(name);
            if (id != 0 && id != uint.MaxValue)
                Bridge.ChangeState(id, "Unlocking");
        }

        private void Close()
        {
            _open = false;
            Scroll_Dialouge_Script.DialogueActive = false;
            if (_narrative != null) _narrative.visible = false;
            if (_canvas    != null) _canvas.active = false;
        }

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
