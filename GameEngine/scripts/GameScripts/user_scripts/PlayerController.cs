using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        [Port("The player's display name")]
        public string playerName = "Player1";

        [Port(0, 200)]
        public int health = 120;

        [Port(50, 300)]
        public int mana = 70;

        public enum States : int {
            Walking, Idle, Attacking
        };

        [Port()]
        public States state = States.Walking;

        public float speed = 5.10f;

        private static InputAction? ia1;
        private static InputAction? ia2;

        public void Begin()
        {
            Console.WriteLine($"ID on script.cs: {this.ID}");

            // Creating a custom input
            ia1 = new InputAction("Jump")
                .AddBinding(KeyCode.Space)                  // Jumps when Space is triggered
                .AddBinding(KeyCode.L, Mod.Control);        // Jumps when L + Shift is triggered

            ia1.when += Jump;
            ia1.when += Jump2;

            ia2 = new InputAction("Sprint")
                .AddBinding(KeyCode.W, Mod.Shift);         // Sprints when W and left shift are pressed

            ia2.when += Sprint;

            ia1.Enable();
            ia2.Enable();

            Console.WriteLine($"End of begin method for entity: {this.ID}");
        }

        public void Update()
        {
            Console.WriteLine("Calling update!");

            // Triggers when releasing A
            if (Input.IsKeyReleased(KeyCode.A))
            {
                Console.WriteLine("Released A!");
            }

            // Triggers when pressing B
            if (Input.IsKeyPressed(KeyCode.Num1))
            {
                Console.WriteLine("Pressed 1!");
            }

            // Triggers when holding C
            if (Input.IsKeyHeld(KeyCode.C))
            {
                Console.WriteLine("Held C!");
            }

            // Testing if the right entity is affected
            // by changing the audio volume the entity is on
            if (Input.IsKeyPressed(KeyCode.V))
            {
                Console.WriteLine("Pressed V, volume should be modified on entity 1 now!");
            }
        }

        private static void Jump()
        {
            Console.WriteLine("Jumping right now!");
        }

        private static void Sprint()
        {
            Console.WriteLine("Sprinting right now!");
        }

        private static void Jump2()
        {
            Console.WriteLine("Jump 2 right now!");
        }
    
		public void _when_node1_started()
        {

        }
    }
}