using System;
using System.Collections.Generic;

namespace Eno
{
	public class BatYellow_Script : ScriptBase
    {
        private Actor _player;

        // How often (seconds) to recompute the A* path
        private const float RecomputeInterval = 0.5f;
        private float _pathTimer = 0f;

        // How close (world units) the bat must be to a waypoint before advancing
        private const float WaypointReachDist = 8f;

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
			