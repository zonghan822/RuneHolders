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
            float dt = Time.deltaTime;

            // Build direction vector from held keys
            float dx = 0f;
            float dy = 0f;

            if (Input.isKeyHeld(KeyCode.W)) dy -= 1f;
            if (Input.isKeyHeld(KeyCode.S)) dy += 1f;
            if (Input.isKeyHeld(KeyCode.A)) dx -= 1f;
            if (Input.isKeyHeld(KeyCode.D)) dx += 1f;

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
        public void Idle_enter()
        {
            // TODO: Add enter logic for idle state
        }

        // Called every frame while in the idle state
        public void Idle_update()
        {
            if (IsMoving())
                _player.changeState("Walk");
        }

        // Called when exiting the idle state
        public void Idle_exit()
        {
            // TODO: Add exit logic for idle state
        }

        // Called when entering the walk state
        public void Walk_enter()
        {
            // TODO: Add enter logic for walk state
        }

        // Called every frame while in the walk state
        public void Walk_update()
        {
            if (!IsMoving())
                _player.changeState("Idle");
        }

        // Called when exiting the walk state
        public void Walk_exit()
        {
            // TODO: Add exit logic for walk state
        }

        private bool IsMoving()
        {
            return Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.S) ||
                   Input.isKeyHeld(KeyCode.A) || Input.isKeyHeld(KeyCode.D);
        }
    
        public void Cast_enter()
        {
            // TODO: Add enter logic for Cast state
        }

        public void Cast_update()
        {
            // TODO: Add update logic for Cast state
        }

        public void Cast_exit()
        {
            // TODO: Add exit logic for Cast state
        }
    
        public void CastWalk_enter()
        {
            // TODO: Add enter logic for CastWalk state
        }

        public void CastWalk_update()
        {
            // TODO: Add update logic for CastWalk state
        }

        public void CastWalk_exit()
        {
            // TODO: Add exit logic for CastWalk state
        }
    
        public void Attack_enter()
        {
            // TODO: Add enter logic for Attack state
        }

        public void Attack_update()
        {
            // TODO: Add update logic for Attack state
        }

        public void Attack_exit()
        {
            // TODO: Add exit logic for Attack state
        }
    
        public void Hurt_enter()
        {
            // TODO: Add enter logic for Hurt state
        }

        public void Hurt_update()
        {
            // TODO: Add update logic for Hurt state
        }

        public void Hurt_exit()
        {
            // TODO: Add exit logic for Hurt state
        }
    
        public void Death_enter()
        {
            // TODO: Add enter logic for Death state
        }

        public void Death_update()
        {
            // TODO: Add update logic for Death state
        }

        public void Death_exit()
        {
            // TODO: Add exit logic for Death state
        }
    }
}
