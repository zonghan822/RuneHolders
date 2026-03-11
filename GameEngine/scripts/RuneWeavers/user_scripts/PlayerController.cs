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
    
        // Called when entering the idle state
        public void idle_enter()
        {
            // TODO: Add enter logic for idle state
        }

        // Called every frame while in the idle state
        public void idle_update()
        {
            if (IsMoving())
                Bridge.ChangeState(ID, "walk");
        }

        // Called when exiting the idle state
        public void idle_exit()
        {
            // TODO: Add exit logic for idle state
        }

        // Called when entering the walk state
        public void walk_enter()
        {
            // TODO: Add enter logic for walk state
        }

        // Called every frame while in the walk state
        public void walk_update()
        {
            if (!IsMoving())
                Bridge.ChangeState(ID, "idle");
        }

        // Called when exiting the walk state
        public void walk_exit()
        {
            // TODO: Add exit logic for walk state
        }

        private bool IsMoving()
        {
            return Bridge.IsKeyHeld(KeyCode.W) || Bridge.IsKeyHeld(KeyCode.S) ||
                   Bridge.IsKeyHeld(KeyCode.A) || Bridge.IsKeyHeld(KeyCode.D);
        }
    }
}
