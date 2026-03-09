using System;
using System.Collections.Generic;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        // Movement speed in world units per second
        public float speed = 300f;

        private Actor _player;

        public void Begin()
        {
            _player = Self<Actor>();

            // ACTOR nodes have physics — keep gravity off for top-down movement
            _player.useGravity  = false;
            _player.isKinematic = false;
        }

        public void Update()
        {
            float dt = Bridge.GetDeltaTime();

            // Build direction vector from held keys
            float dx = 0f;
            float dy = 0f;

            if (Bridge.IsKeyHeld(KeyCode.W)) dy -= 1f;
            if (Bridge.IsKeyHeld(KeyCode.S)) dy += 1f;
            if (Bridge.IsKeyHeld(KeyCode.A)) dx -= 1f;
            if (Bridge.IsKeyHeld(KeyCode.D)) dx += 1f;

            // Normalize diagonal movement so speed is consistent in all directions
            if (dx != 0f && dy != 0f)
            {
                float inv = 1f / MathF.Sqrt(dx * dx + dy * dy);
                dx *= inv;
                dy *= inv;
            }

            _player.velocity = new Vector2(dx * speed, dy * speed);
        }
    
        // Called when entering the default state

        // Called every frame while in the default state

        // Called when exiting the default state
    }
}
