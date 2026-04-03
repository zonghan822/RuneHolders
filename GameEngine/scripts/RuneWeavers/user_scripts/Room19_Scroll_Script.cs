using System;

namespace Eno
{
    public class Room19_Scroll_Script : ScriptBase
    {
        private Canvas?   _canvas;
        private Sprite2D? _narrative;
        private bool _shown = false;
        private bool _open  = false;
        private bool _skipE = false;
        private uint _scrollPopUpID  = 0;
        private bool _prevController = false;

        public void Begin()
        {
            try { _canvas    = GetNode<Canvas>("Dialogue Canvas"); } catch { }
            try { _narrative = GetNode<Sprite2D>("Narrative");      } catch { }
            if (_canvas != null) _canvas.active = false;
            _shown = false;
            _open  = false;
            _skipE = false;

            _scrollPopUpID  = Bridge.GetNode("Room19_ScrollPopUp");
            _prevController = Input.inputMode == InputMode.Controller;
            if (_scrollPopUpID != 0)
                Bridge.SetTexturePath(_scrollPopUpID, _prevController ? "UI-PopUp-A" : "UI-PopUp-E");
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;
            if (isController != _prevController)
            {
                _prevController = isController;
                if (_scrollPopUpID != 0)
                    Bridge.SetTexturePath(_scrollPopUpID, isController ? "UI-PopUp-A" : "UI-PopUp-E");
            }

            uint playerID = Bridge.GetNode("Player");
            if (playerID == 0 || playerID == uint.MaxValue) return;

            if (_open && (Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.B))))
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
            _shown = _open = true;
            _skipE = true;
            Scroll_Dialouge_Script.DialogueActive = true;
            if (_narrative != null) { _narrative.texture = "UI-Narrative-2"; _narrative.visible = true; }
            if (_canvas    != null) _canvas.active = true;
            Bridge.SetButtonActive(Bridge.GetNode("NarrativeBackButton"), true);
            UnlockDoor("Door7Ato7");
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
