using System;
using System.Runtime.InteropServices;

namespace Eno
{
	public class EnemyController : ScriptBase
    {

        private string _lastState = "";

        public void OnStateChanged(string newState)
        {
            // Call the C++ animation system
            Bridge.SetEntityAnimation(ID, newState);
        }

        // Or map states to animations if names differ
        public void SetAnimationForState(string state)
        {
            // might need this incase 2 names correspond with the same animation
            string animName = state switch
            {
                "Chase" => "Walk",
                "Windup" => "Attack",
                _ => state  // Idle, Attack, Hurt, Death map 1:1
            };

            Bridge.SetEntityAnimation(ID, animName);
        }

        public void Begin()
        {
            //Console.WriteLine("Enemy controller Begin invoked!");
            Console.WriteLine($"[C#] EnemyController Begin for entity {ID}");
            _lastState = "";
        }

        public void Update()
        {
            // Uncomment below if you want to see Update spam
            // Console.WriteLine("Enemy controller Update invoked!");

            // Poll the AI state from C++
            string currentState = Bridge.GetAIState(ID);

            // Check if state changed
            if (currentState != _lastState && !string.IsNullOrEmpty(currentState))
            {
                Console.WriteLine($"[C#] Entity {ID}: State changed from '{_lastState}' to '{currentState}'");

                // Map state to animation name
                string animName = currentState switch
                {
                    "Chase" => "Walk",
                    "Windup" => "Attack",
                    _ => currentState  // Idle, Attack, Hurt, Death map 1:1
                };

                Console.WriteLine($"[C#] Entity {ID}: Setting animation to '{animName}'");
                bool success = Bridge.SetEntityAnimation(ID, animName);
                Console.WriteLine($"[C#] Entity {ID}: SetEntityAnimation returned {success}");

                _lastState = currentState;
            }

        }

        // ==================== State Callbacks ====================
        // These are called by C++ when AI state changes
        // Set useScriptCallbacks=true and cbXxxEnter="OnXxxEnter" in AIComponent

        public void OnIdleEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => IDLE");
            Bridge.SetEntityAnimation(ID, "Idle");
        }

        public void OnChaseEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => CHASE (setting Walk animation)");
            Bridge.SetEntityAnimation(ID, "Walk");
        }

        public void OnWindupEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => WINDUP (setting Attack animation)");
            Bridge.SetEntityAnimation(ID, "Attack");
        }

        public void OnAttackEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => ATTACK");
            Bridge.SetEntityAnimation(ID, "Attack");
        }

        public void OnHurtEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => HURT");
            Bridge.SetEntityAnimation(ID, "Hurt");
        }

        public void OnDeathEnter()
        {
            Console.WriteLine($"[C#] Entity {ID} => DEATH");
            Bridge.SetEntityAnimation(ID, "Death");
        }

		public void _when_node1_started()
        {
            Console.WriteLine("Node1 has started with audio playing!");
        }
	}

}




