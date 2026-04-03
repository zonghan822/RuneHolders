using System;

namespace Eno
{
    public class Room17_Scroll_Script : ScriptBase
    {
        private Canvas?   _canvas;
        private Sprite2D? _narrative;
        private bool _shown = false;
        private bool _open  = false;
        private bool _skipE = false;
        private AudioNode? _doorAudio = null;

        public void Begin()
        {
            try { _canvas    = GetNode<Canvas>("Dialogue Canvas"); } catch { }
            try { _narrative = GetNode<Sprite2D>("Narrative");      } catch { }
            if (_canvas != null) _canvas.active = false;
            _shown = false;
            _open  = false;
            _skipE = false;
            _doorAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
        }

        public void Update()
        {
            uint playerID = Bridge.GetNode("Player");
            if (playerID == 0 || playerID == uint.MaxValue) return;

            if (_open && (Input.isKeyPressed(KeyCode.Escape) || Input.isGamepadButtonPressed(GamepadButton.B)))
            {
                Close();
                return;
            }

            bool ePressed = (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A)) && !_skipE;
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
            _shown = _open = true;
            _skipE = true;
            Scroll_Dialouge_Script.DialogueActive = true;
            if (_narrative != null) { _narrative.texture = "UI-Narrative-1"; _narrative.visible = true; }
            if (_canvas    != null) _canvas.active = true;
            UnlockDoor("Door5Ato5");
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
