namespace Eno
{
    public class Room4_Script : ScriptBase
    {
        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;

        private bool  _scrollPickedUp = false;

        private Canvas?    _videoCanvas = null;
        private VideoNode? _videoHolder = null;

        private uint  _uiInfoID    = 0;
        private float _uiInfoTimer = 0f;

        public void Begin()
        {
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room4_Scroll");
            _exitDoorID  = Bridge.GetNode("Door4to5");
            _spellbookID = Bridge.GetNode("Spellbook");
            _uiInfoID    = Bridge.GetNode("UI_Info");

            try { _videoCanvas = GetNode<Canvas>("VideoCanvas");    } catch { }
            try { _videoHolder = GetNode<VideoNode>("VideoHolder"); } catch { }

            _scrollPickedUp = false;
            _uiInfoTimer    = 0f;
        }

        public void Update()
        {
            if (_uiInfoID != 0 && _uiInfoTimer < 3f)
            {
                _uiInfoTimer += Time.deltaTime;
                if (_uiInfoTimer >= 3f)
                    Bridge.SetVisibility(_uiInfoID, false);
            }

            // E on scroll → pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[1] &&
                Input.isKeyPressed(KeyCode.E) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                // Hide the scroll
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                // Show F-to-open hint
                if (_uiInfoID != 0)
                {
                    Bridge.SetTexturePath(_uiInfoID, "UI-FtoOpen-Text");
                    Bridge.SetVisibility(_uiInfoID, true);
                }
                _uiInfoTimer = 0f;
                // Update spellbook
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-2Spells");
                if (_exitDoorID  != 0) Bridge.ChangeState(_exitDoorID, "Unlocking");
                // Play video
                if (_videoCanvas != null) _videoCanvas.active = true;
                if (_videoHolder != null) _videoHolder.play();
            }
        }
    }
}
