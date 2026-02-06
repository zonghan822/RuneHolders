using System;
using System.Collections.Generic;

namespace Eno
{
	public class TrailBlight_Script : ScriptBase
    {
        // TrailBlight AI parameters (loaded from AIComponent)
        private float awarenessRadius;
        private float chaseSpeed;
        private float attackRange;
        private float windupTime;
        private float attackDuration;
        private float hurtDuration;
        private int health;
        private bool allowDiagonal;

        private float attackTimer = 0.0f;
        private float stateTimer = 0.0f;

        public void Begin()
        {
            Console.WriteLine($"[TrailBlight] Initializing TrailBlight entity {ID}");

            // Load AI parameters from the AIComponent
            if (Bridge.GetAIComponent(ID, out awarenessRadius, out chaseSpeed, out attackRange,
                out windupTime, out attackDuration, out hurtDuration, out health, out allowDiagonal))
            {
                Console.WriteLine($"[TrailBlight {ID}] Loaded AI params - Chase: {chaseSpeed}, Attack Range: {attackRange}, Awareness: {awarenessRadius}");
            }
            else
            {
                Console.WriteLine($"[TrailBlight {ID}] ERROR: Failed to load AIComponent! Using defaults.");
                // Fallback defaults
                awarenessRadius = 200.0f;
                chaseSpeed = 90.0f;
                attackRange = 40.0f;
                windupTime = 0.5f;
                attackDuration = 0.3f;
                hurtDuration = 0.4f;
                health = 100;
                allowDiagonal = true;
            }
		}

        public void Update()
        {
            // Get actual delta time from engine
            float deltaTime = Bridge.GetDeltaTime();

            // Decay timers
            if (attackTimer > 0.0f)
            {
                attackTimer -= deltaTime;
            }
            if (stateTimer > 0.0f)
            {
                stateTimer -= deltaTime;
            }
		}

        // Called when entering the idle state
        public void idle_enter()
        {
            Bridge.SetEntityAnimation(ID, "idle");

            // Stop movement when entering idle
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);
            Bridge.ClearPath(ID);
        }

        // Called every frame while in the idle state
        public void idle_update()
        {
            // Check if player is in awareness range
            if (Bridge.GetPlayerPosition(out float playerX, out float playerY))
            {
                if (Bridge.GetEntityPosition(ID, out float myX, out float myY))
                {
                    float distance = Bridge.CalculateDistance(myX, myY, playerX, playerY);

                    if (distance < awarenessRadius)
                    {
                        // Player is in range, start chasing
                        Console.WriteLine($"[TrailBlight {ID}] Player detected! Distance: {distance:F1}");
                        Bridge.ChangeState(ID, "chase");
                    }
                }
            }
        }

        // Called when exiting the idle state
        public void idle_exit()
        {
            Console.WriteLine($"[TrailBlight {ID}] Leaving idle state");
        }

        // Called when entering the chase state
        public void chase_enter()
        {
            Bridge.SetEntityAnimation(ID, "chase");
            Console.WriteLine($"[TrailBlight {ID}] Entering chase state");
        }

        // Called every frame while in the chase state
        public void chase_update()
        {
            // Get player position
            if (!Bridge.GetPlayerPosition(out float playerX, out float playerY))
            {
                // Player not found, return to idle
                Console.WriteLine($"[TrailBlight {ID}] Lost player, returning to idle");
                Bridge.ChangeState(ID, "idle");
                return;
            }

            if (!Bridge.GetEntityPosition(ID, out float myX, out float myY))
            {
                return;
            }

            float distanceToPlayer = Bridge.CalculateDistance(myX, myY, playerX, playerY);

            // Check if player is out of awareness range (with hysteresis)
            if (distanceToPlayer > awarenessRadius * 1.2f)
            {
                Console.WriteLine($"[TrailBlight {ID}] Player too far, returning to idle");
                Bridge.ChangeState(ID, "idle");
                return;
            }

            // Check if in attack range
            if (distanceToPlayer < attackRange)
            {
                Console.WriteLine($"[TrailBlight {ID}] Player in attack range!");
                Bridge.ChangeState(ID, "windup");
                return;
            }

            // Request pathfinding to player position
            bool pathRequested = Bridge.RequestPathToPosition(ID, playerX, playerY, allowDiagonal);

            if (pathRequested)
            {
                // Get next waypoint
                if (Bridge.GetNextWaypoint(ID, out float waypointX, out float waypointY))
                {
                    // Move towards waypoint
                    Bridge.MoveTowards(ID, waypointX, waypointY, chaseSpeed);
                }
                else
                {
                    // No path available, move directly towards player
                    Bridge.MoveTowards(ID, playerX, playerY, chaseSpeed);
                }
            }
            else
            {
                // Pathfinding failed, move directly towards player
                Bridge.MoveTowards(ID, playerX, playerY, chaseSpeed);
            }
        }

        // Called when exiting the chase state
        public void chase_exit()
        {
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);
            Bridge.ClearPath(ID);
            Console.WriteLine($"[TrailBlight {ID}] Exiting chase state");
        }

        // Called when entering the windup state
        public void windup_enter()
        {
            Bridge.SetEntityAnimation(ID, "windup");

            // Stop movement during windup
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);
            stateTimer = windupTime;
            Console.WriteLine($"[TrailBlight {ID}] Winding up for attack ({windupTime}s)");
        }

        // Called every frame while in the windup state
        public void windup_update()
        {
            // Transition to attack after windup duration
            if (stateTimer <= 0.0f)
            {
                Bridge.ChangeState(ID, "attack");
            }
        }

        // Called when exiting the windup state
        public void windup_exit()
        {
            Console.WriteLine($"[TrailBlight {ID}] Windup complete");
        }

        // Called when entering the attack state
        public void attack_enter()
        {
            Bridge.SetEntityAnimation(ID, "attack");

            Console.WriteLine($"[TrailBlight {ID}] Attacking!");
            // Stop movement during attack
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);

            // TODO: Trigger attack animation, spawn projectile, deal damage, etc.
            // This would typically involve additional bridge functions

            // Set attack duration timer
            stateTimer = attackDuration;
        }

        // Called every frame while in the attack state
        public void attack_update()
        {
            // Transition back to chase after attack completes
            if (stateTimer <= 0.0f)
            {
                Bridge.ChangeState(ID, "chase");
            }
        }

        // Called when exiting the attack state
        public void attack_exit()
        {
            Console.WriteLine($"[TrailBlight {ID}] Attack finished");
        }

        // Called when entering the hurt state
        public void hurt_enter()
        {
            Bridge.SetEntityAnimation(ID, "hurt");
            Console.WriteLine($"[TrailBlight {ID}] Ouch! Taking damage!");
            // Stop movement when hurt
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);

            // Set hurt duration timer
            stateTimer = hurtDuration;

            // TODO: Play hurt animation, apply knockback, etc.
        }

        // Called every frame while in the hurt state
        public void hurt_update()
        {
            // Transition back to chase after hitstun recovery
            if (stateTimer <= 0.0f)
            {
                Bridge.ChangeState(ID, "chase");
            }
        }

        // Called when exiting the hurt state
        public void hurt_exit()
        {
            Console.WriteLine($"[TrailBlight {ID}] Recovering from hurt");
        }

        // Called when entering the death state
        public void death_enter()
        {
            Bridge.SetEntityAnimation(ID, "death");
            Console.WriteLine($"[TrailBlight {ID}] Defeated!");
            // Stop all movement
            Bridge.SetEntityVelocity(ID, 0.0f, 0.0f);
            Bridge.ClearPath(ID);

            // Start death timer
            stateTimer = 1.0f; // 1 seconds for death animation
        }

        // Called every frame while in the death state
        public void death_update()
        {
            // Decrement death timer
            float deltaTime = Bridge.GetDeltaTime();
            stateTimer -= deltaTime;

            // Wait for death animation to complete, then destroy self
            if (stateTimer <= 0.0f)
            {
                Console.WriteLine($"[TrailBlight {ID}] Death animation complete, destroying entity");
                Bridge.DestroyEntity(ID);
            }
        }

        // Called when exiting the death state
        public void death_exit()
        {
            // This typically won't be called as entity is destroyed in death state
            Console.WriteLine($"[TrailBlight {ID}] Death state exited");
        }
	}
}