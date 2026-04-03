using System;
using System.Collections.Generic;

namespace Eno
{
	public class Narrative_Script : ScriptBase
    {
        private uint _narrativeID  = 0;
        private uint _canvasID     = 0;
        private uint _backButtonID = 0;

        public void Begin()
        {
            _narrativeID  = Bridge.GetNode("Narrative");
            _canvasID     = Bridge.GetNode("Dialogue Canvas");
            _backButtonID = Bridge.GetNode("NarrativeBackButton");
		}

        public void Update()
        {

		}

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }

		public void _when_narrativebackbutton_onclick()
        {
            if (_backButtonID != 0) Bridge.SetTexturePath(_backButtonID, "UI-Back");
            Scroll_Dialouge_Script.DialogueActive = false;
            if (_narrativeID  != 0) Bridge.SetVisibility(_narrativeID, false);
            if (_canvasID     != 0)
            {
                SetChildButtonsActive(_canvasID, false);
                Bridge.SetCanvasActive(_canvasID, false);
            }
        }

        private void SetChildButtonsActive(uint parentID, bool active)
        {
            uint count = Bridge.GetChildCount(parentID);
            for (uint i = 0; i < count; i++)
            {
                uint childID = Bridge.GetChild(parentID, i);
                if (Bridge.GetNodeType(childID) == (uint)NodeType.BUTTON)
                    Bridge.SetButtonActive(childID, active);
                SetChildButtonsActive(childID, active);
            }
        }

		public void _when_narrativebackbutton_onpressed()
        {
            if (_backButtonID != 0)
                Bridge.SetTexturePath(_backButtonID, "UI-Back-Selected");
        }
	}
}
