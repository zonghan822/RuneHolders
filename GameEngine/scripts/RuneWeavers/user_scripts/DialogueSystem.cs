using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Eno
{
    public class DialogueSystem : ScriptBase
    {
        [Port(0.01f, 0.2f)]
        public float typeSpeed = 0.03f;

        private string[]  lines_       = Array.Empty<string>();
        private int       index_       = -1;
        private bool      active_      = false;
        private string    target_      = "";
        private string    current_     = "";
        private float     timer_       = 0f;
        private bool      typing_      = false;
        private uint      boxEntityID_ = 0;

        public void Begin()
        {
            boxEntityID_ = Bridge.FindEntityByName("DialogueBox");
            string? assetsDir = Marshal.PtrToStringUni(Bridge.GetAssetsDir());
            if (assetsDir != null)
            {
                string path = Path.Combine(assetsDir, "settings", "dialogue.txt");
                if (File.Exists(path))
                    lines_ = File.ReadAllLines(path);
            }
            if (boxEntityID_ != 0)
                Bridge.SetEntityVisible(boxEntityID_, false);
        }

        public void Update()
        {
            if (!active_)
                return;

            if (typing_)
            {
                timer_ += Bridge.GetDeltaTime();
                while (timer_ >= typeSpeed && current_.Length < target_.Length)
                {
                    current_ += target_[current_.Length];
                    timer_   -= typeSpeed;
                }
                if (boxEntityID_ != 0)
                    Bridge.SetEntityText(boxEntityID_, current_);
                if (current_.Length == target_.Length)
                    typing_ = false;
            }

            if (Bridge.IsKeyPressed(KeyCode.E))
                Advance();
        }

        private void Advance()
        {
            if (typing_)
            {
                // Skip to end of current line
                current_ = target_;
                typing_  = false;
                if (boxEntityID_ != 0)
                    Bridge.SetEntityText(boxEntityID_, current_);
                return;
            }

            index_++;
            if (index_ >= lines_.Length)
            {
                End();
                return;
            }

            target_  = lines_[index_];
            current_ = "";
            timer_   = 0f;
            typing_  = true;
        }

        // Called from C++ via coreHost_.runScriptMethod(L"Eno.DialogueSystem", L"StartDialogue")
        public void StartDialogue()
        {
            if (lines_.Length == 0)
                return;

            // Re-find the dialogue box in case the scene changed since Begin()
            if (boxEntityID_ == 0)
                boxEntityID_ = Bridge.FindEntityByName("DialogueBox");

            index_  = -1;
            active_ = true;
            if (boxEntityID_ != 0)
                Bridge.SetEntityVisible(boxEntityID_, true);
            Bridge.PauseGame();
            Advance();
        }

        private void End()
        {
            active_ = false;
            if (boxEntityID_ != 0)
                Bridge.SetEntityVisible(boxEntityID_, false);
            Bridge.ResumeGame();
        }
    }
}
