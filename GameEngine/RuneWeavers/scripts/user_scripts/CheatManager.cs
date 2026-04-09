using System;

namespace Eno
{
    public class CheatManager : ScriptBase
    {
        public static bool GodMode = false;

        private Label? _cheatLabel = null;
        private float  _msgTimer   = 0f;
        private const float MSG_DURATION = 2f;

        public void Begin()
        {
            GodMode = false;
            uint id = Bridge.GetNode("UIText");
            if (id != 0 && id != uint.MaxValue)
            {
                _cheatLabel = NodeFactory.Create(id) as Label;
                Bridge.SetVisibility(_cheatLabel!.ID, false);
            }
        }

        private void ShowMsg(string text)
        {
            if (_cheatLabel == null) return;
            _cheatLabel.text = text;
            Bridge.SetVisibility(_cheatLabel.ID, true);
            _msgTimer = MSG_DURATION;
        }

        public void Update()
        {
            if (_cheatLabel != null && _msgTimer > 0f)
            {
                _msgTimer -= Time.deltaTime;
                if (_msgTimer <= 0f)
                    Bridge.SetVisibility(_cheatLabel.ID, false);
            }

            bool lt = Bridge.GetGamepadAxis((int)GamepadAxis.LeftTrigger) > 0f;

            // F1 / LT+DPadUp: Kill all enemies
            if (Input.isKeyPressed(KeyCode.F1) || (lt && Input.isGamepadButtonPressed(GamepadButton.DPadUp)))
            {
                foreach (Enemy enemy in GetAllEnemies())
                    enemy.health = 0f;
                ShowMsg("Cheat: Kill All Enemies");
            }

            // F2 / LT+DPadRight: Skip to end (teleport to room 14 area)
            if (Input.isKeyPressed(KeyCode.F2) || (lt && Input.isGamepadButtonPressed(GamepadButton.DPadRight)))
            {
                uint playerID = Bridge.GetNode("Player");
                uint cameraID = Bridge.GetNode("Camera");
                if (playerID != 0 && playerID != uint.MaxValue)
                {
                    Bridge.SetPosition(playerID, 8640f, -8969f);
                    Bridge.SetVelocity(playerID, 0f, 0f);
                }
                if (cameraID != 0 && cameraID != uint.MaxValue)
                    Bridge.SetPosition(cameraID, 8638f, -9218f);

                RuneSystem.SpellUnlocked[0] = true;
                RuneSystem.SpellUnlocked[1] = true;
                RuneSystem.SpellUnlocked[2] = true;
                Room1_Script.StopUI();
                Room2_Script.StopUI();
                Room4_Script.StopUI();
                Room8_Script.StopUI();
                ShowMsg("Cheat: Skip To End");
            }

            // F3 / LT+DPadDown: Toggle god mode
            if (Input.isKeyPressed(KeyCode.F3) || (lt && Input.isGamepadButtonPressed(GamepadButton.DPadDown)))
            {
                GodMode = !GodMode;
                ShowMsg("Cheat: God Mode " + (GodMode ? "ON" : "OFF"));
            }

            // Keep player at full health when god mode is active
            if (GodMode)
            {
                uint playerID = Bridge.GetNode("Player");
                if (playerID != 0 && playerID != uint.MaxValue)
                    Bridge.SetHealth(playerID, Bridge.GetMaxHealth(playerID));
            }

            // F4 / LT+DPadLeft: Unlock all spells
            if (Input.isKeyPressed(KeyCode.F4) || (lt && Input.isGamepadButtonPressed(GamepadButton.DPadLeft)))
            {
                RuneSystem.SpellUnlocked[0] = true;
                RuneSystem.SpellUnlocked[1] = true;
                RuneSystem.SpellUnlocked[2] = true;
                ShowMsg("Cheat: All Spells Unlocked");
            }

        }

        public void Default_enter()  { }
        public void Default_update() { }
        public void Default_exit()   { }
    }
}
