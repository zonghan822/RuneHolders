using System;

namespace Eno
{
    public class DoorScript : ScriptBase
    {
        // How close the player must be to trigger the door
        [Port(10, 200)]
        public float triggerRadius = 60f;

        // Where the camera should pan to (world coordinates)
        [Port("Target camera X position")]
        public float targetCamX = 2000f;

        [Port("Target camera Y position")]
        public float targetCamY = 3300f;

        // Where the player should be teleported after the camera finishes panning
        [Port("Player destination X")]
        public float destPlayerX = 2000f;

        [Port("Player destination Y")]
        public float destPlayerY = 3594f;

        // How fast the camera pans (pixels per second)
        [Port(100, 5000)]
        public float panSpeed = 1500f;

        // Internal state
        private bool isTransitioning = false;
        private bool hasTriggered = false;

        public void Begin()
        {
        }

        public void Update()
        {
            if (isTransitioning)
            {
                PanCamera();
                return;
            }

            if (hasTriggered) return;

            // Check if player is near the door
            if (!Bridge.GetPlayerPosition(out float playerX, out float playerY)) return;
            if (!Bridge.GetEntityPosition(this.ID, out float doorX, out float doorY)) return;

            float dist = Bridge.CalculateDistance(playerX, playerY, doorX, doorY);

            if (dist < triggerRadius)
            {
                Bridge.PlayInstantAudio("SFX", "SpellCast_01");
                isTransitioning = true;
                hasTriggered = true;
                Console.WriteLine("[Door] Player entered door - starting camera pan");
            }
        }

        private void PanCamera()
        {
            if (!Bridge.GetCameraPosition(out float camX, out float camY)) return;

            float dx = targetCamX - camX;
            float dy = targetCamY - camY;
            float dist = (float)Math.Sqrt(dx * dx + dy * dy);

            float dt = Bridge.GetDeltaTime();
            float step = panSpeed * dt;

            if (dist <= step)
            {
                // Camera arrived - snap to target and teleport the player
                Bridge.SetCameraPosition(targetCamX, targetCamY);
                Bridge.SetPlayerPosition(destPlayerX, destPlayerY);

                isTransitioning = false;
                Console.WriteLine("[Door] Transition complete");
            }
            else
            {
                // Move camera towards target at constant speed
                float nx = dx / dist;
                float ny = dy / dist;
                Bridge.SetCameraPosition(camX + nx * step, camY + ny * step);
            }
        }
    }
}
