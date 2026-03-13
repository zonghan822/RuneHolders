using System;
using System.Collections.Generic;

namespace Eno
{
	public class Scroll_Dialouge_Script : ScriptBase
    {
        // ------------------- Dialogue configuration ------------------------
        private static readonly List<string> dialogueTexts = new List<string>
        {
            "OH, WHAT'S THIS?",
            "A SCROLL FULL OF SPELLS!",
            "HEHEHEHE...",
            "DID I JUST HEAR SOMETHING?",
            "WH-WHO GOES THERE?",
            "...",
            "I GUESS I WAS JUST HEARING THINGS THEN.",
            "LET ME RECORD THE SPELLS IN MY SPELLBOOK."
        };

        private const float DIALOGUE_TYPE_SPEED = 0.05f;

        // Set this in the editor to the scene name of the door to unlock when dialogue ends.
        [Port] public string DoorName = "";
        // Max characters per line before wrapping to the next line.
        [Port] public int CharsPerLine = 30;

        // Node references
        private Actor  _player;
        private Node2D _scroll;
        private Canvas _dialogueCanvas;
        private Label  _dialogueText;

        // Set to true once the final dialogue line is dismissed; other scripts can read this.
        public static bool DialogueComplete = false;

        // Runtime state
        private bool   dialogueActive      = false;
        private bool   dialogueTyping      = false;
        private int    dialogueIndex       = 0;
        private string dialogueCurrentText = "";
        private string dialogueTargetText  = "";
        private float  dialogueTypeTimer   = 0f;

        private static readonly Random rng = new Random();

        public void Begin()
        {
            _player         = GetNode<Actor>("Player");
            _scroll         = GetNode<Node2D>("Room1_Scroll");
            _dialogueCanvas = GetNode<Canvas>("Dialogue Canvas");
            _dialogueText   = GetNode<Label>("Dialogue Text");

            if (_dialogueCanvas != null) _dialogueCanvas.active = false;
        }

        public void Update()
        {
            // Trigger dialogue when player enters scroll zone and presses E
            if (!dialogueActive)
            {
                if (_scroll != null && _player != null &&
                    Bridge.IsColliding(_scroll.ID, _player.ID) &&
                    Input.isKeyPressed(KeyCode.E))
                {
                    StartDialogue();
                }
                return;
            }

            // E key: skip typewriter or advance to next line 
            if (Input.isKeyPressed(KeyCode.E))
            {
                if (dialogueTyping)
                {
                    // Finish current animation instantly
                    dialogueTyping      = false;
                    dialogueCurrentText = dialogueTargetText;
                    if (_dialogueText != null)
                        _dialogueText.text = dialogueCurrentText;
                }
                else
                {
                    AdvanceDialogue();
                }
            }

            // Typewriter animation 
            if (dialogueTyping)
            {
                dialogueTypeTimer += Time.deltaTime;

                while (dialogueTypeTimer >= DIALOGUE_TYPE_SPEED &&
                       dialogueCurrentText.Length < dialogueTargetText.Length)
                {
                    dialogueCurrentText += dialogueTargetText[dialogueCurrentText.Length];
                    dialogueTypeTimer   -= DIALOGUE_TYPE_SPEED;

                    // Play a random typing sound (Typing_01 … Typing_21)
                    int    sfxNum  = rng.Next(1, 22);
                    string sfxName = "Typing_" + (sfxNum < 10 ? "0" : "") + sfxNum;
                    try { GetNode<AudioNode>(sfxName)?.play(); } catch { }
                }

                if (_dialogueText != null)
                    _dialogueText.text = dialogueCurrentText;

                if (dialogueCurrentText.Length >= dialogueTargetText.Length)
                    dialogueTyping = false;
            }
        }

        private void StartDialogue()
        {
            if (dialogueTexts.Count == 0) return;

            dialogueActive = true;
            dialogueIndex  = 0;

            if (_dialogueCanvas != null)
                _dialogueCanvas.active = true;

            if (_dialogueText != null)
                _dialogueText.text = "";

            dialogueTargetText  = WrapText(dialogueTexts[0], CharsPerLine);
            dialogueCurrentText = "";
            dialogueTypeTimer   = 0f;
            dialogueTyping      = true;
        }

        private void AdvanceDialogue()
        {
            // Next line
            dialogueIndex++;
            // If reached last line, unlock the door to go to the next room
            if (dialogueIndex >= dialogueTexts.Count)
            {
                dialogueActive = false;
                dialogueTyping = false;
                // Set Canvas Inactive
                if (_dialogueCanvas != null) _dialogueCanvas.active = false;
                DialogueComplete = true;

                if (!string.IsNullOrEmpty(DoorName))
                {
                    uint doorID = Bridge.GetNode(DoorName);
                    if (doorID != 0)
                    {
                        Bridge.SetVisibility(doorID, false);
                        Bridge.SetPosition(doorID, -99999f, -99999f);
                    }
                }
            }
            else
            {
                if (_dialogueText != null)
                    _dialogueText.text = "";

                dialogueTargetText  = WrapText(dialogueTexts[dialogueIndex], CharsPerLine);
                dialogueCurrentText = "";
                dialogueTypeTimer   = 0f;
                dialogueTyping      = true;
            }
        }

        private string WrapText(string text, int maxChars)
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

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
	}
}
