namespace Eno
{
    public class Room8_Script : ScriptBase
    {
        private uint _scrollID    = 0;
        private uint _playerID    = 0;
        private uint _spellbookID = 0;
        private uint _exitDoorID  = 0;

        private bool  _postDialogueActivated = false;
        private bool  _scrollPickedUp        = false;

        private Canvas?    _videoCanvas = null;
        private VideoNode? _videoHolder = null;

        public void Begin()
        {
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room8_Scroll");
            _exitDoorID  = Bridge.GetNode("Door8to9");
            _spellbookID = Bridge.GetNode("Spellbook");

            try { _videoCanvas = GetNode<Canvas>("VideoCanvas");    } catch { }
            try { _videoHolder = GetNode<VideoNode>("VideoHolder"); } catch { }
        }

        public void Update()
        {
            // After dialogue ends: activate ui sprite, then hide after 3 seconds
            if (!_postDialogueActivated && Scroll_Dialouge_Script.DialogueShown[2])
            {
                _postDialogueActivated = true;
            }

            // E on scroll → pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Scroll_Dialouge_Script.DialogueShown[2] &&
                Input.isKeyPressed(KeyCode.E) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                // Hide the scroll
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                // Update spellbook
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "UI-Spellbook-3Spells");
                if (_exitDoorID  != 0) Bridge.ChangeState(_exitDoorID, "Unlocking");
                // Play video
                if (_videoCanvas != null) _videoCanvas.active = true;
                if (_videoHolder != null) _videoHolder.play();
            }
        }
    }
}
