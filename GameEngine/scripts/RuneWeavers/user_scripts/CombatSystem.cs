using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Eno
{
    public class CombatSystem : LogicSystem
    {
        [Port(1f, 500f)]
        public float playerHealth = 100f;

        [Port(1f, 500f)]
        public float enemyHealth  = 50f;

        [Port(0f, 200f)]
        public float enemyDamage  = 10f;

        public void Begin()
        {
            string? assetsDir = Marshal.PtrToStringUni(Bridge.GetAssetsDir());
            if (assetsDir != null)
            {
                string cfg = Path.Combine(assetsDir, "settings", "combat_config.txt");
                if (File.Exists(cfg))
                    LoadConfig(cfg);
            }

            uint player = Bridge.FindEntityByName("Player");
            if (player != 0)
                Bridge.SetEntityHealth(player, playerHealth);
        }

        public void Update()
        {
            // Per-frame death check for the player
            uint player = Bridge.FindEntityByName("Player");
            if (player != 0 && Bridge.GetEntityHealth(player) <= 0f)
                Bridge.LoadScene("LoseScreen.txt");
        }

        private void LoadConfig(string path)
        {
            foreach (var line in File.ReadAllLines(path))
            {
                var parts = line.Split('=');
                if (parts.Length != 2) continue;
                string key = parts[0].Trim();
                if (float.TryParse(parts[1].Trim(), out float val))
                {
                    if      (key == "player_health") playerHealth = val;
                    else if (key == "enemy_health")  enemyHealth  = val;
                    else if (key == "enemy_damage")  enemyDamage  = val;
                }
            }
        }
    }
}
