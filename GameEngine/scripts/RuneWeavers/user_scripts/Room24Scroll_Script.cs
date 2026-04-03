using System;
using System.Collections.Generic;
using static Eno.Scroll_Dialouge_Script;

namespace Eno
{
    public class Room24Scroll_Script : ScriptBase
    {
        private static readonly List<DialogueLine> _dialogue = new List<DialogueLine>
        {
            new DialogueLine(Speaker.WitchMain,  Emotion.Default, Emotion.SadGrey,      "We're here... my room."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Default, Emotion.WideEyedGrey, "I already asked you if you want your horns gone..."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Default, Emotion.WideEyedGrey, "But from what I've seen of your capabilities"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Angry,   Emotion.WideEyedGrey, "You don't really mean it, do you?"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Angry,   Emotion.WideEyedGrey, "You learn and execute the spells I left for you so fast"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Default, Emotion.WideEyedGrey, "against creatures you've never seen before too."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Angry,   Emotion.WideEyedGrey, "Do you truly want your skill and talent to go to waste?"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Angry,   Emotion.WideEyedGrey, "And more importantly... your love for magic?"),
            new DialogueLine(Speaker.CassiaMain, Emotion.Sad,     Emotion.AngryGrey,    "I..."),
            new DialogueLine(Speaker.CassiaMain, Emotion.Sad,     Emotion.AngryGrey,    "No... I don't! I don't want that!"),
            new DialogueLine(Speaker.CassiaMain, Emotion.Sad,     Emotion.AngryGrey,    "But... my friends... school... the whole world...!"),
            new DialogueLine(Speaker.CassiaMain, Emotion.Cry,     Emotion.AngryGrey,    "NO ONE ACCEPTS ME!"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Smile,   Emotion.WideEyedGrey, "But I do."),
            new DialogueLine(Speaker.CassiaMain, Emotion.Sad,     Emotion.SmileGrey,    "Wha...?"),
            new DialogueLine(Speaker.WitchMain,  Emotion.Smile,   Emotion.SadGrey,      "It was nice having someone to talk to after 200 years."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Smile,   Emotion.WideEyedGrey, "And I want to talk more... about you, magic, the world now..."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Smile,   Emotion.WideEyedGrey, "So what I'm saying is... instead of ridding of your horns and magic..."),
            new DialogueLine(Speaker.WitchMain,  Emotion.Smile,   Emotion.WideEyedGrey, "Will you be my first friend?"),
        };

        private const float TYPE_SPEED    = 0.05f;
        private const int   CHARS_PER_LINE = 30;

        // Node references
        private Canvas?   _canvas;
        private Label?    _dialogueText;
        private Sprite2D? _dialogueBox;
        private Sprite2D? _charCenter;
        private Sprite2D? _charLeft;
        private Sprite2D? _charRight;
        private AudioNode? _typingSfx;
        
        // Win screen
        private uint _winLoseCanvasID = 0;
        private uint _victoryScreenID = 0;

        // Runtime state
        private bool   _triggered   = false;
        private bool   _active      = false;
        private bool   _skipE       = false;
        private bool   _typing      = false;
        private int    _lineIndex   = 0;
        private string _currentText = "";
        private string _targetText  = "";
        private float  _typeTimer   = 0f;


        public void Begin()
        {
            try { _canvas        = GetNode<Canvas>("Dialogue Canvas");  } catch { }
            try { _dialogueText  = GetNode<Label>("Dialogue Text");     } catch { }
            try { _dialogueBox   = GetNode<Sprite2D>("Dialogue Box");   } catch { }
            try { _charCenter    = GetNode<Sprite2D>("CharacterCenter"); } catch { }
            try { _charLeft      = GetNode<Sprite2D>("ProtagonistLeft"); } catch { }
            try { _charRight     = GetNode<Sprite2D>("WitchRight");     } catch { }
            try { _typingSfx     = GetNode<AudioNode>("Typing SFX");   } catch { }
            try { _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");   } catch { }
            try { _victoryScreenID = Bridge.GetNode("WinScreen");       } catch { }

            if (_canvas != null) _canvas.active = false;

            _triggered   = false;
            _active      = false;
            _skipE       = false;
            _typing      = false;
            _lineIndex   = 0;
            _currentText = "";
            _targetText  = "";
            _typeTimer   = 0f;
        }

        public void Update()
        {
            bool isController = Input.inputMode == InputMode.Controller;

            if (!_active)
            {
                if (_triggered) return;

                uint playerID = Bridge.GetNode("Player");
                if (playerID == 0 || playerID == uint.MaxValue) return;

                if (!Scroll_Dialouge_Script.DialogueActive
                    && Bridge.IsColliding(ID, playerID)
                    && (Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.A))))
                {
                    StartDialogue();
                }
                return;
            }

            // Escape or B button closes dialogue
            if (Input.isKeyPressed(KeyCode.Escape) || (isController && Input.isGamepadButtonPressed(GamepadButton.B)))
            {
                CloseDialogue();
                return;
            }

            bool ePressed = (Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.A))) && !_skipE;
            _skipE = false;

            if (ePressed)
            {
                if (_typing)
                {
                    // Skip typewriter — show full line instantly
                    _typing      = false;
                    _currentText = _targetText;
                    if (_dialogueText != null) _dialogueText.text = _currentText;
                }
                else
                {
                    AdvanceDialogue();
                }
            }

            // Typewriter
            if (_typing)
            {
                _typeTimer += Time.deltaTime;

                while (_typeTimer >= TYPE_SPEED && _currentText.Length < _targetText.Length)
                {
                    char c = _targetText[_currentText.Length];
                    _currentText += c;
                    _typeTimer   -= TYPE_SPEED;

                    if (c != ' ' && c != '\n' && _typingSfx != null)
                        Bridge.AudioPlay(_typingSfx.ID);
                }

                if (_dialogueText != null) _dialogueText.text = _currentText;

                if (_currentText.Length >= _targetText.Length)
                    _typing = false;
            }
        }

        private void StartDialogue()
        {
            _triggered = true;
            _active    = true;
            _skipE     = true;
            Scroll_Dialouge_Script.DialogueActive = true;

            if (_canvas      != null) _canvas.active = true;
            if (_dialogueBox != null) _dialogueBox.visible = true;
            if (_dialogueText != null) { _dialogueText.visible = true; _dialogueText.text = ""; }

            _lineIndex = 0;
            ShowLine(_dialogue[0]);
        }

        private void AdvanceDialogue()
        {
            _lineIndex++;
            if (_lineIndex >= _dialogue.Count)
                CloseDialogue();
            else
                ShowLine(_dialogue[_lineIndex]);
        }

        private void CloseDialogue()
        {
            _active = false;
            Scroll_Dialouge_Script.DialogueActive = false;

            if (_canvas      != null) _canvas.active = false;
            if (_dialogueBox != null) _dialogueBox.visible = true;
            if (_charCenter  != null) _charCenter.visible  = false;
            if (_charLeft    != null) _charLeft.visible    = false;
            if (_charRight   != null) _charRight.visible   = false;

            // Show win screen
            if (_winLoseCanvasID != 0)
            {
                Bridge.SetCanvasActive(_winLoseCanvasID, true);
                if (_victoryScreenID != 0)
                {
                    PlayerController.IsGameCompleted = true;
                    Bridge.SetVisibility(_victoryScreenID, true);
                    SetChildButtonsActive(_victoryScreenID, true);
                }
            }

        }

        private void ShowLine(DialogueLine line)
        {
            if (_charCenter != null) _charCenter.visible = false;
            if (_charLeft   != null) _charLeft.visible   = false;
            if (_charRight  != null) _charRight.visible  = false;

            switch (line._speaker)
            {
                case Speaker.Cassia:
                    if (_charCenter != null)
                    {
                        _charCenter.texture = DetermineCharacterTex(line._speaker, line._emotion);
                        _charCenter.visible = true;
                    }
                    if (_dialogueBox != null) _dialogueBox.texture = "UI-DialogueBox-Cassia";
                    break;

                case Speaker.Witch:
                    if (_charCenter != null)
                    {
                        _charCenter.texture = DetermineCharacterTex(line._speaker, line._emotion);
                        _charCenter.visible = true;
                    }
                    if (_dialogueBox != null) _dialogueBox.texture = "UI-DialogueBox-Witch";
                    break;

                case Speaker.CassiaMain:
                    if (_charLeft != null)
                    {
                        _charLeft.texture = DetermineCharacterTex(line._speaker, line._emotion);
                        _charLeft.visible  = true;
                    }
                    if (_charRight != null && line._greyEmotion != Emotion.None)
                    {
                        _charRight.texture = DetermineCharacterTex(line._speaker, line._greyEmotion);
                        _charRight.visible  = true;
                    }
                    if (_dialogueBox != null) _dialogueBox.texture = "UI-DialogueBox-Cassia";
                    break;

                case Speaker.WitchMain:
                    if (_charLeft != null && line._greyEmotion != Emotion.None)
                    {
                        _charLeft.texture = DetermineCharacterTex(line._speaker, line._greyEmotion);
                        _charLeft.visible  = true;
                    }
                    if (_charRight != null)
                    {
                        _charRight.texture = DetermineCharacterTex(line._speaker, line._emotion);
                        _charRight.visible  = true;
                    }
                    if (_dialogueBox != null) _dialogueBox.texture = "UI-DialogueBox-Witch";
                    break;
            }

            if (_dialogueBox  != null) _dialogueBox.visible = true;
            if (_dialogueText != null) _dialogueText.text   = "";

            _targetText  = WrapText(line._line, CHARS_PER_LINE);
            _currentText = "";
            _typeTimer   = 0f;
            _typing      = true;
        }

        private static string DetermineCharacterTex(Speaker speaker, Emotion emotion)
        {
            bool isGrey = emotion.ToString().EndsWith("Grey");
            bool useProtagonist = (speaker == Speaker.Cassia || speaker == Speaker.CassiaMain)
                                  ? !isGrey
                                  : isGrey;
            if (useProtagonist) return "Protagonist-Dialogue-" + emotion.ToString();
            return "Witch-Dialogue-" + emotion.ToString();
        }

        private static string WrapText(string text, int maxChars)
        {
            string[] words  = text.Split(' ');
            string   result = "";
            string   line   = "";

            foreach (string word in words)
            {
                if (line.Length + word.Length + (line.Length > 0 ? 1 : 0) > maxChars)
                {
                    result += (result.Length > 0 ? "\n" : "") + line;
                    line    = word;
                }
                else
                {
                    line += (line.Length > 0 ? " " : "") + word;
                }
            }

            if (line.Length > 0)
                result += (result.Length > 0 ? "\n" : "") + line;

            return result;
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

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
