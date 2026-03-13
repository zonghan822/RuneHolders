namespace Eno
{
    public class Room1_Script : ScriptBase
    {
        private uint _scrollID       = 0;
        private uint _playerID       = 0;
        private uint _activeSpriteID = 0;  // TODO: replace "ACTIVATION_SPRITE_PLACEHOLDER"
        private uint _spellbookID    = 0;  // TODO: replace "SPELLBOOK_ENTITY_PLACEHOLDER"
        private uint _exitDoorID     = 0;

        private bool _dialogueTriggered     = false;
        private bool _postDialogueActivated = false;
        private bool _scrollPickedUp        = false;

        public void Begin()
        {
            _playerID    = Bridge.GetNode("Player");
            _scrollID    = Bridge.GetNode("Room1_Scroll");
            _exitDoorID  = Bridge.GetNode("Door1to2");
            _activeSpriteID = Bridge.GetNode("ACTIVATION_SPRITE_PLACEHOLDER");
            _spellbookID    = Bridge.GetNode("SPELLBOOK_ENTITY_PLACEHOLDER");
        }

        public void Update()
        {
            // Trigger Room 1 dialogue on first frame
            if (!_dialogueTriggered)
            {
                _dialogueTriggered = true;
                Scroll_Dialouge_Script.TriggerDialogue(0);
            }

            // After dialogue ends: activate ui sprite
            if (!_postDialogueActivated && Scroll_Dialouge_Script.DialogueShown[0])
            {
                _postDialogueActivated = true;
                if (_activeSpriteID != 0) Bridge.SetVisibility(_activeSpriteID, true);
            }

            // E on scroll → pickup: scroll disappears, spellbook updates, door unlocks
            if (!_scrollPickedUp && Input.isKeyPressed(KeyCode.E) &&
                _scrollID != 0 && _playerID != 0 &&
                Bridge.IsColliding(_scrollID, _playerID))
            {
                _scrollPickedUp = true;
                Bridge.SetVisibility(_scrollID, false);
                Bridge.SetColliderActive(_scrollID, false);
                if (_spellbookID != 0) Bridge.SetTexturePath(_spellbookID, "SPRITESHEET_PLACEHOLDER");
                if (_exitDoorID  != 0) Bridge.ChangeState(_exitDoorID, "Unlocking");
            }
        }
    }
}
