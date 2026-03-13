using System;
using System.Collections.Generic;

namespace Eno
{
	public class Scroll_Dialouge_Script : ScriptBase
    {
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
        private static readonly List<string>[] _dialogueSets =
        {
            new List<string>
            {
                "It really is here... the rumoured mansion.",
                "A human... faun?",
                "Wait...are you-",
                "Dang, they ran away. But that should be...",
                "Huh, they dropped a scroll... for a spell?",
                "Let me record it in my SPELLBOOK."
            },   // Room1_Scroll

            new List<string> 
            { 
                "Finally caught up to you!",
                "WAIT I mean no harm, I promise!",
                "I'm here because...",
                "I... I want my horns gone.",
                "Everyone at school hates me, bullies me and-",
                "not even see me as human...",
                "Just because of these horns on my head...!",
                "Rumours say that there's a witch in the Elden Forest-",
                "that can rid one of their monster features...",
                "You're that witch right?! Please...",
                "Please help me get rid of them and let me live a proper life!",
                "......",
                "If you're that determined...",
                "Follow me.",
            },   // Room4_Scroll

            new List<string> 
            { 
                "......",
                "Do you truly want your horns gone?",
                "I...",
                "You are well aware of the cose of the spell, right?",
                "You... can no longer cast magic anymore.",
                "... I'm well aware.",
                "... Only a few more rooms left",
                "Come.",
            },   // Room8_Scroll

            new List<string> 
            { 
                "We're here... my room.",
                "I already asked you if you want your horns gone...",
                "But from what I've seen of your capabilities",
                "You dont't really mean it, do you?",
                "You learn and execute the spells I left for you so fast",
                "against creatures you've never seen before too.",
                "Do you truly want your skill and talent to go to waste?",
                "And more importantly... your love for magic?",
                "I...",
                "No... I don't! I don't want that!",
                "But... my friends... school... the whole world...!",
                "NO ONE ACCEPTS ME!",
                "But I do.",
                "Wha...?",
                "It was nice having someone to talk to after 200 years.",
                "And I want to talk more... about you, magic, the world now...",
                "So what I'm saying is... instead of ridding of your horns and magic...",
                "Will you be my first friend?",
            },   // Room24_Scroll
        };

        private const float DIALOGUE_TYPE_SPEED = 0.05f;

        // Max characters per line before word-wrap.
        [Port] public int CharsPerLine = 30;

        // ---------- Static state readable by other scripts --------
        private static Scroll_Dialouge_Script? _instance;
        public  static bool DialogueActive = false;

        // ------------------- Node references ----------------------------
        private Actor?    _player;
        private Canvas?   _dialogueCanvas;
        private Label?    _dialogueText;
        private Node2D?   _narrativeSprite;
        private Node2D?   _dialogueBox;
        private Node2D?[] _scrollNodes = Array.Empty<Node2D?>();
        private AudioNode? _typingSfx;

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
            try { _narrativeSprite = GetNode<Node2D>("Narrative"); } catch { }
            try { _dialogueBox    = GetNode<Node2D>("Dialogue Box"); } catch { }

            _scrollNodes = new Node2D?[ScrollNames.Length];
            for (int i = 0; i < ScrollNames.Length; i++)
                try { _scrollNodes[i] = GetNode<Node2D>(ScrollNames[i]); } catch { }

            if (_dialogueCanvas != null) _dialogueCanvas.active = false;

            try { _typingSfx = GetNode<AudioNode>("Typing SFX"); } catch { }
        }

        // Called by PlayerController when the player interacts with a scroll.
        public static void TriggerDialogue(int scrollIndex)
        {
            if (_instance == null || DialogueActive) return;
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
                        Bridge.AudioStop(_typingSfx.ID);
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
                Console.WriteLine($"[Dialogue] Narrative index {index}: texture = {_narrativeTextures[index]}");
                if (_narrativeSprite != null)
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

                dialogueIndex       = 0;
                dialogueTargetText  = WrapText(_dialogueSets[index][0], CharsPerLine);
                dialogueCurrentText = "";
                dialogueTypeTimer   = 0f;
                dialogueTyping      = true;
                Console.WriteLine($"[Dialogue] Text index {index}, line 0: \"{_dialogueSets[index][0]}\"");
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
                // Reset typewriter animation values and input next line
                Console.WriteLine($"[Dialogue] Text index {_activeIndex}, line {dialogueIndex}: \"{_dialogueSets[_activeIndex][dialogueIndex]}\"");
                if (_dialogueText != null) _dialogueText.text = "";
                dialogueTargetText  = WrapText(_dialogueSets[_activeIndex][dialogueIndex], CharsPerLine);
                dialogueCurrentText = "";
                dialogueTypeTimer   = 0f;
                dialogueTyping      = true;
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

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
	}
}
