using System;
using System.Collections.Generic;

namespace Eno
{
	public class SlugYellow_Script : ScriptBase
    {
        private Actor _player;

        // How often (seconds) to recompute the A* path
        private const float RecomputeInterval = 0.5f;
        private float _pathTimer = 0f;

        // How close (world units) the bat must be to a waypoint before advancing
        private const float WaypointReachDist = 8f;

        // Contact damage: 20% of player max HP per hit, 1-second i-frames
        private const float ContactDamagePercent = 0.2f;
        private const float ContactIFrameDuration = 1.0f;

        public void Begin()
        {
            _player = GetNode<Actor>("Player");
            Bridge.SetUseGravity(ID, false);
		}

        public void Update()
        {

		}

        private float DistToPlayer()
        {
            float dx = _player.position.x - Bridge.GetPositionX(ID);
            float dy = _player.position.y - Bridge.GetPositionY(ID);
            return MathF.Sqrt(dx * dx + dy * dy);
        }

        public void Spawn_enter()
        {
            // TODO: Add enter logic for Spawn state
        }

        public void Spawn_update()
        {
            Bridge.ChangeState(ID, "Idle");
        }

        public void Spawn_exit()
        {
            // TODO: Add exit logic for Spawn state
        }

        public void Idle_enter()
        {
            Bridge.SetVelocity(ID, 0f, 0f);
        }

        public void Idle_update()
        {
            if (_player == null) return;

            if (DistToPlayer() <= Bridge.GetAwarenessRadius(ID))
                Bridge.ChangeState(ID, "Chase");
        }

        public void Idle_exit()
        {
            // TODO: Add exit logic for Idle state
        }

        public void Chase_enter()
        {
            _pathTimer = RecomputeInterval; // trigger immediate compute on first update
        }

        public void Chase_update()
        {
            if (_player == null) return;

            if (DistToPlayer() > Bridge.GetAwarenessRadius(ID))
            {
                Bridge.ChangeState(ID, "Idle");
                return;
            }

            // Recompute path on a timer so the bat tracks a moving player
            _pathTimer += Bridge.GetDeltaTime();
            if (_pathTimer >= RecomputeInterval)
            {
                _pathTimer = 0f;
                float myX = Bridge.GetPositionX(ID);
                float myY = Bridge.GetPositionY(ID);
                Bridge.AI_ComputePath(ID, myX, myY, _player.position.x, _player.position.y);
            }

            if (!Bridge.AI_HasPath(ID))
            {
                Bridge.SetVelocity(ID, 0f, 0f);
                return;
            }

            // Steer toward the current waypoint
            float wpX = Bridge.AI_GetNextWaypointX(ID);
            float wpY = Bridge.AI_GetNextWaypointY(ID);
            float posX = Bridge.GetPositionX(ID);
            float posY = Bridge.GetPositionY(ID);
            float dx = wpX - posX;
            float dy = wpY - posY;
            float dist = MathF.Sqrt(dx * dx + dy * dy);

            if (dist <= WaypointReachDist)
            {
                Bridge.AI_Advance(ID);
                return;
            }

            float speed = Bridge.GetChaseSpeed(ID);
            Bridge.SetVelocity(ID, dx / dist * speed, dy / dist * speed);

            // Deal contact damage if touching the player
            if (Bridge.IsColliding(ID, _player.ID))
                EnemyCombat.TryDealContactDamage(_player.ID, ContactDamagePercent, ContactIFrameDuration);
        }

        public void Chase_exit()
        {
            Bridge.AI_ClearPath(ID);
            Bridge.SetVelocity(ID, 0f, 0f);
        }
	
        public void WindUp_enter()
        {
            // TODO: Add enter logic for WindUp state
        }

        public void WindUp_update()
        {
            // TODO: Add update logic for WindUp state
        }

        public void WindUp_exit()
        {
            // TODO: Add exit logic for WindUp state
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
	
        public void Frozen_enter()
        {
            // TODO: Add enter logic for Frozen state
        }

        public void Frozen_update()
        {
            // TODO: Add update logic for Frozen state
        }

        public void Frozen_exit()
        {
            // TODO: Add exit logic for Frozen state
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
			