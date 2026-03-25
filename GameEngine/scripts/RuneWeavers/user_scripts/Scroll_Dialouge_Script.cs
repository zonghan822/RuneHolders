using System;
using System.Collections.Generic;

namespace Eno
{
	public class Scroll_Dialouge_Script : ScriptBase
    {
        // Enums for speaker
        public enum Speaker
        {
            Cassia,
            Witch,
            CassiaMain,
            WitchMain,
        };

        public enum Emotion
        {
            Default,
            Cry,
            CryGrey,
            DefaultGrey,
            Sad,
            SadGrey,
            Surprised,
            SurprisedGrey,
            WideEyed,
            WideEyedGrey,
            Angry,
            AngryGrey,
            Smile,
            SmileGrey,
            None,
        }

        // Single set of dialogue
        public struct DialogueLine
        {
            public Speaker _speaker;
            public Emotion _emotion;
            public Emotion _greyEmotion;
            public string _line;

            // Two characters on screen
            public DialogueLine(Speaker speaker, Emotion emotion, Emotion greyEmotion, string line)
            {
                _speaker = speaker; _emotion = emotion; _greyEmotion = greyEmotion; _line = line;
            }
            // Single character on screen
            public DialogueLine(Speaker speaker, Emotion emotion, string line)
            {
                _speaker = speaker; _emotion = emotion; _greyEmotion = Emotion.None; _line = line;
            }
        }

        // ------------------- Scroll registry ----------------------------
        // All interactable scroll/guide entity names, in index order.
        // PlayerController uses this same array for collision checks.
        public static readonly string[] ScrollNames =
        {
            "Room1_Scroll", "Room4_Scroll", "Room8_Scroll", "Room24_Scroll",
            "Room10_Scroll", "Room14_Scroll", "Room17_Scroll", "Room19_Scroll"
        };

        // true = show [Narrative] image, false = run typewriter text
        private static readonly bool[] _isNarrative =
        {
            false, false, false, false,
            true,  true,  true,  true
        };

        // Texture name for narrative scrolls (unused for text scrolls)
        private static readonly string[] _narrativeTextures =
        {
            "", "", "", "",
            "UI-Narrative-1", "UI-Narrative-2", "UI-Narrative-3", "UI-Narrative-4"
        };

        // Text dialogue sets
        private static readonly List<DialogueLine>[] _dialogueSets =
        {
            new List<DialogueLine>
            {
                new DialogueLine(Speaker.Cassia,        Emotion.Default,                            "It really is here... the rumoured mansion."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.DefaultGrey,     "A human... faun?"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Surprised,  Emotion.None,            "Wait...are you-"),
                new DialogueLine(Speaker.Cassia,        Emotion.Sad,                                "Dang, they ran away. But that should be..."),
                new DialogueLine(Speaker.Cassia,        Emotion.WideEyed,                           "Huh, they dropped a scroll... for a spell?"),
                new DialogueLine(Speaker.Cassia,        Emotion.Default,                            "Let me record it in my SPELLBOOK.")
            },   // Room1_Scroll

            new List<DialogueLine>
            {
                new DialogueLine(Speaker.CassiaMain,    Emotion.WideEyed,   Emotion.DefaultGrey,     "WAIT I mean no harm, I promise!"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Surprised,  Emotion.DefaultGrey,     "Finally caught up to you!"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Surprised,  Emotion.DefaultGrey,     "I'm here because..."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.DefaultGrey,     "I... I want my horns gone."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.DefaultGrey,     "Everyone at school hates me, bullies me and-"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.DefaultGrey,     "not even see me as human..."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.DefaultGrey,     "Just because of these horns on my head...!"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.DefaultGrey,     "Rumours say that there's a witch in the Elden Forest-"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.DefaultGrey,     "that can rid one of their monster features..."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.DefaultGrey,     "You're that witch right?! Please..."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.DefaultGrey,     "Please help me get rid of them and let me live a proper life!"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SadGrey,         "......"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SadGrey,         "If you're that determined..."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SurprisedGrey,   "Follow me.")
            },   // Room4_Scroll

            new List<DialogueLine>
            {
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.DefaultGrey,       "......"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.WideEyedGrey,      "Do you truly want your horns gone?"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.DefaultGrey,       "I..."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SadGrey,           "You are well aware of the cost of the spell, right?"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SadGrey,           "You... can no longer cast magic anymore."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.DefaultGrey,       "... I'm well aware."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.CryGrey,           "... Only a few more rooms left"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.CryGrey,           "Come.")
            },   // Room8_Scroll

            new List<DialogueLine>
            {
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.SadGrey,           "We're here... my room."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.WideEyedGrey,      "I already asked you if you want your horns gone..."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.WideEyedGrey,      "But from what I've seen of your capabilities"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Angry,      Emotion.WideEyedGrey,      "You don't really mean it, do you?"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Angry,      Emotion.WideEyedGrey,      "You learn and execute the spells I left for you so fast"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Default,    Emotion.WideEyedGrey,      "against creatures you've never seen before too."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Angry,      Emotion.WideEyedGrey,      "Do you truly want your skill and talent to go to waste?"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Angry,      Emotion.WideEyedGrey,      "And more importantly... your love for magic?"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.AngryGrey,         "I..."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.AngryGrey,         "No... I don't! I don't want that!"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.AngryGrey,         "But... my friends... school... the whole world...!"),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Cry,        Emotion.AngryGrey,         "NO ONE ACCEPTS ME!"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Smile,      Emotion.WideEyedGrey,      "But I do."),
                new DialogueLine(Speaker.CassiaMain,    Emotion.Sad,        Emotion.SmileGrey,         "Wha...?"),
                new DialogueLine(Speaker.WitchMain,     Emotion.Smile,      Emotion.SadGrey,           "It was nice having someone to talk to after 200 years."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Smile,      Emotion.WideEyedGrey,      "And I want to talk more... about you, magic, the world now..."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Smile,      Emotion.WideEyedGrey,      "So what I'm saying is... instead of ridding of your horns and magic..."),
                new DialogueLine(Speaker.WitchMain,     Emotion.Smile,      Emotion.WideEyedGrey,      "Will you be my first friend?")
            }
        };

        private const float DIALOGUE_TYPE_SPEED = 0.05f;

        // Max characters per line before word-wrap.
        [Port] public int CharsPerLine = 30;

        // ---------- Static state readable by other scripts --------
        private static Scroll_Dialouge_Script? _instance;
        public  static bool   DialogueActive = false;
        public  static bool[] DialogueShown  = new bool[ScrollNames.Length];

        // ------------------- Node references ----------------------------
        private Actor?    _player;
        private Canvas?   _dialogueCanvas;
        private Label?    _dialogueText;
        private Node2D?   _narrativeSprite;
        private Node2D?   _dialogueBox;
        private Node2D?[] _scrollNodes = Array.Empty<Node2D?>();
        private AudioNode? _typingSfx;

        // Character display nodes
        private Node2D? _charCenter;        // Centre character image
        private Node2D? _charLeft;          // Cassia on the left
        private Node2D? _charRight;         // Witch on the right

        // Win screen node IDs
        private uint _winLoseCanvasID = 0;
        private uint _victoryScreenID = 0;

        // ------------------- Runtime state ------------------------------
        private bool   dialogueActive      = false;
        private bool   _skipEInput         = false;
        private bool   dialogueTyping      = false;
        private int    dialogueIndex       = 0;
        private int    _activeIndex        = -1;
        private string dialogueCurrentText = "";
        private string dialogueTargetText  = "";
        private float  dialogueTypeTimer   = 0f;

        public void Begin()
        {
            _instance       = this;
            _player         = GetNode<Actor>("Player");
            _dialogueCanvas = GetNode<Canvas>("Dialogue Canvas");
            _dialogueText   = GetNode<Label>("Dialogue Text");
            _narrativeSprite = GetNode<Node2D>("Narrative");
            try { _dialogueBox    = GetNode<Node2D>("Dialogue Box"); } catch { }

            _scrollNodes = new Node2D?[ScrollNames.Length];
            for (int i = 0; i < ScrollNames.Length; i++)
                try { _scrollNodes[i] = GetNode<Node2D>(ScrollNames[i]); } catch { }

            if (_dialogueCanvas != null) _dialogueCanvas.active = false;

            try { _typingSfx = GetNode<AudioNode>("Typing SFX"); } catch { }

            try { _charCenter  = GetNode<Node2D>("CharacterCenter");  } catch { }
            try { _charLeft    = GetNode<Node2D>("ProtagonistLeft");    } catch { }
            try { _charRight   = GetNode<Node2D>("WitchRight");   } catch { }

            try { _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas"); } catch { }
            try { _victoryScreenID = Bridge.GetNode("WinScreen");  } catch { }

            // Reset static state so dialogue replays on level restart
            DialogueActive = false;
            DialogueShown  = new bool[ScrollNames.Length];

            // Reset instance state
            dialogueActive      = false;
            _skipEInput         = false;
            dialogueTyping      = false;
            dialogueIndex       = 0;
            _activeIndex        = -1;
            dialogueCurrentText = "";
            dialogueTargetText  = "";
            dialogueTypeTimer   = 0f;
        }

        // Called by room scripts when the player interacts with a text-based scroll.
        public static void TriggerDialogue(int scrollIndex)
        {
            if (_instance == null || DialogueActive || DialogueShown[scrollIndex]) return;
            if (_isNarrative[scrollIndex]) return;
            _instance.StartDialogueForScroll(scrollIndex);
        }

        // Called by room scripts when the player interacts with a narrative scroll.
        public static void TriggerNarrative(int scrollIndex)
        {
            if (_instance == null || DialogueActive || DialogueShown[scrollIndex]) return;
            if (!_isNarrative[scrollIndex]) return;
            Console.WriteLine("NARRATIVE");
            _instance.StartDialogueForScroll(scrollIndex);
        }

        public void Update()
        {
            if (!dialogueActive) return;

            if (Input.isKeyPressed(KeyCode.Escape))
            {
                CloseDialogue();
                return;
            }

            if (Input.isKeyPressed(KeyCode.E) && !_skipEInput)
            {
                if (_activeIndex >= 0 && _isNarrative[_activeIndex])
                {
                    CloseDialogue();
                    return;
                }

                if (dialogueTyping)
                {
                    // Skip typewriter — show full line instantly
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
            _skipEInput = false;

            // Typewriter animation
            if (dialogueTyping)
            {
                dialogueTypeTimer += Time.deltaTime;

                while (dialogueTypeTimer >= DIALOGUE_TYPE_SPEED &&
                       dialogueCurrentText.Length < dialogueTargetText.Length)
                {
                    char c = dialogueTargetText[dialogueCurrentText.Length];
                    dialogueCurrentText += c;
                    dialogueTypeTimer   -= DIALOGUE_TYPE_SPEED;

                    // Play typing sound for each non-whitespace character
                    if (c != ' ' && c != '\n' && _typingSfx != null)
                    {
                        //Bridge.AudioStop(_typingSfx.ID);
                        Bridge.AudioPlay(_typingSfx.ID);
                    }
                }

                if (_dialogueText != null)
                    _dialogueText.text = dialogueCurrentText;

                if (dialogueCurrentText.Length >= dialogueTargetText.Length)
                    dialogueTyping = false;
            }
        }

        private void StartDialogueForScroll(int index)
        {
            _skipEInput    = true;
            DialogueActive = true;
            dialogueActive = true;
            _activeIndex   = index;

            if (_dialogueCanvas != null) _dialogueCanvas.active = true;
            // Check if current scroll is for dialogue or narrative
            if (_isNarrative[index])
            {
                // Show narrative sprite, hide text label and dialogue box
                //if (_narrativeSprite != null)
                {
                    Bridge.SetTexturePath(_narrativeSprite.ID, _narrativeTextures[index]);
                    Bridge.SetVisibility(_narrativeSprite.ID, true);
                }
                if (_dialogueText != null)
                    Bridge.SetVisibility(_dialogueText.ID, false);
                if (_dialogueBox != null)
                    Bridge.SetVisibility(_dialogueBox.ID, false);

                dialogueTyping = false;
            }
            else
            {
                // Show text label and dialogue box, hide narrative sprite
                if (_narrativeSprite != null)
                    Bridge.SetVisibility(_narrativeSprite.ID, false);
                if (_dialogueBox != null)
                    Bridge.SetVisibility(_dialogueBox.ID, true);
                if (_dialogueText != null)
                {
                    Bridge.SetVisibility(_dialogueText.ID, true);
                    _dialogueText.text = "";
                }

                dialogueIndex = 0;
                ShowLine(_dialogueSets[index][0]);
            }
        }

        private void AdvanceDialogue()
        {
            // Next line of text
            dialogueIndex++;

            // If last text, close dialogue
            if (dialogueIndex >= _dialogueSets[_activeIndex].Count)
            {
                CloseDialogue();
            }
            else
            {
                ShowLine(_dialogueSets[_activeIndex][dialogueIndex]);
            }
        }

        private void CloseDialogue()
        {
            // Set flags to false
            dialogueActive = false;
            dialogueTyping = false;
            DialogueActive = false;
            // Set canvas inactive
            if (_dialogueCanvas  != null) _dialogueCanvas.active = false;
            if (_narrativeSprite != null) Bridge.SetVisibility(_narrativeSprite.ID, false);
            if (_dialogueBox     != null) Bridge.SetVisibility(_dialogueBox.ID, true);
            // Hide character display nodes
            if (_charCenter  != null) Bridge.SetVisibility(_charCenter.ID,  false);
            if (_charLeft    != null) Bridge.SetVisibility(_charLeft.ID,    false);
            if (_charRight   != null) Bridge.SetVisibility(_charRight.ID,   false);
            // To prevent dialogue from repeating
            if (_activeIndex >= 0 && !_isNarrative[_activeIndex]) DialogueShown[_activeIndex] = true;

            // Final dialogue (Room24_Scroll, index 3) — show the win screen
            if (_activeIndex == 3 && _winLoseCanvasID != 0)
            {
                Bridge.SetCanvasActive(_winLoseCanvasID, true);
                if (_victoryScreenID != 0)
                {
                    Bridge.SetVisibility(_victoryScreenID, true);
                    SetChildButtonsActive(_victoryScreenID, true);
                }
            }
        }

        // Displays one dialogue line: sets character sprites, speaker name texture, and starts typewriter.
        private void ShowLine(DialogueLine line)
        {
            // Hide all character nodes first
            if (_charCenter != null) Bridge.SetVisibility(_charCenter.ID, false);
            if (_charLeft   != null) Bridge.SetVisibility(_charLeft.ID,   false);
            if (_charRight  != null) Bridge.SetVisibility(_charRight.ID,  false);

            // Identify speaker and current emotion
            switch (line._speaker)
            { 
                case Speaker.Cassia:
                    if (_charCenter != null)
                    {
                        Bridge.SetTexturePath(_charCenter.ID, determineCharacterTex(line._speaker, line._emotion));
                        Bridge.SetVisibility(_charCenter.ID, true);
                    }
                    // Change dialogue box texture
                    if (_dialogueBox != null)
                        Bridge.SetTexturePath(_dialogueBox.ID, "UI-DialogueBox-Cassia");
                    break;

                case Speaker.Witch:
                    if (_charCenter != null)
                    {
                        Bridge.SetTexturePath(_charCenter.ID, determineCharacterTex(line._speaker, line._emotion));
                        Bridge.SetVisibility(_charCenter.ID, true);
                    }
                    // Change dialogue box texture
                    if (_dialogueBox != null)
                        Bridge.SetTexturePath(_dialogueBox.ID, "UI-DialogueBox-Witch");
                    break;

                case Speaker.CassiaMain:
                    // Cassia ACTIVE on left, Witch GREY on right
                    if (_charLeft != null)
                    {
                        Bridge.SetTexturePath(_charLeft.ID, determineCharacterTex(line._speaker, line._emotion));
                        Bridge.SetVisibility(_charLeft.ID, true);
                    }
                    if (_charRight != null)
                    {
                        if (line._greyEmotion != Emotion.None)
                        {
                            Bridge.SetTexturePath(_charRight.ID, determineCharacterTex(line._speaker, line._greyEmotion));
                            Bridge.SetVisibility(_charRight.ID, true);
                        }
                        // else stays hidden
                    }
                    // Change dialogue box texture
                    if (_dialogueBox != null)
                        Bridge.SetTexturePath(_dialogueBox.ID, "UI-DialogueBox-Cassia");
                    break;

                case Speaker.WitchMain:
                    // Cassia GREY on left, Witch ACTIVE on right
                    if (_charLeft != null)
                    {
                        if (line._greyEmotion != Emotion.None)
                        {
                            Bridge.SetTexturePath(_charLeft.ID, determineCharacterTex(line._speaker, line._greyEmotion));
                            Bridge.SetVisibility(_charLeft.ID, true);
                        }
                        // else stays hidden
                    }
                    if (_charRight != null)
                    {
                        Bridge.SetTexturePath(_charRight.ID, determineCharacterTex(line._speaker, line._emotion));
                        Bridge.SetVisibility(_charRight.ID, true);
                    }
                    // Change dialogue box texture
                    if (_dialogueBox != null)
                        Bridge.SetTexturePath(_dialogueBox.ID, "UI-DialogueBox-Witch");
                    break;
            }

            // Show dialogue box with current speaker name
            if (_dialogueBox != null)
                Bridge.SetVisibility(_dialogueBox.ID, true);

            // Start typewriter
            if (_dialogueText != null) _dialogueText.text = "";
            dialogueTargetText  = WrapText(line._line, CharsPerLine);
            dialogueCurrentText = "";
            dialogueTypeTimer   = 0f;
            dialogueTyping      = true;
        }


        // Helper function to create new line of text to prevent overflow out of dialogue box
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


        // Returns the asset texture name for a given speaker + emotion.
        private static string determineCharacterTex(Speaker speaker, Emotion emotion)
        {
            // Determine which character is the main speaker
            bool isGrey = emotion.ToString().EndsWith("Grey");

            bool useProtagonist = (speaker == Speaker.Cassia || speaker == Speaker.CassiaMain)
                                  ? !isGrey   // Cassia active
                                  : isGrey;   // Witch active

            // If using Cassia
            if (useProtagonist) return "Protagonist-Dialogue-" + emotion.ToString();

            // Using Witch
            return "Witch-Dialogue-" + emotion.ToString();
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
