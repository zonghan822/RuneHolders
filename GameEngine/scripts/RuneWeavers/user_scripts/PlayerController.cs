using System;
using System.Collections.Generic;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        // Movement speed in world units per second
        public float speed = 300f;

        // Duration of the attack animation in seconds
        public float attackDuration = 0.5f;

        private Actor _player;
        private float _attackTimer  = 0f;
        private bool  _isAttacking  = false;

        public void Begin()
        {
            _player = Self<Actor>();

            // ACTOR nodes have physics — keep gravity off for top-down movement
            _player.useGravity  = false;
            _player.isKinematic = false;
            
        }

        public void Update()
        {
            if (_isAttacking) return;

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

            // Flip sprite horizontally based on horizontal direction
            if (dx != 0f)
            {
                Vector2 s = _player.scale;
                _player.scale = new Vector2(dx > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
            }
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
        }

        public void Cast_update()
        {
            if (IsMoving())
                _player.changeState("CastWalk");
        }

        public void Cast_exit()
        {
        }
    
        public void CastWalk_enter()
        {
        }

        public void CastWalk_update()
        {
            if (!IsMoving())
                _player.changeState("Cast");
        }

        public void CastWalk_exit()
        {
        }
    
        public void Attack_enter()
        {
            _isAttacking = true;
            _attackTimer = 0f;
            _player.velocity = new Vector2(0f, 0f);
            _player.animation = "Attack";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(attackDuration / frames);
        }

        public void Attack_update()
        {
            _attackTimer += Time.deltaTime;
            if (_attackTimer >= attackDuration)
                _player.changeState("Idle");
        }

        public void Attack_exit()
        {
            _isAttacking = false;
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
