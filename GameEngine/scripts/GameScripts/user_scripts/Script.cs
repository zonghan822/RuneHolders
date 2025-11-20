using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        [Reveal("The player's display name")]
        public string playerName = "Player1";

        [Reveal(0, 200)]
        public int health = 120;

        [Reveal(50, 300)]
        public int mana = 70;

        public float speed = 5.10f;

        private static InputAction? ia1;
        private static InputAction? ia2;

        public static void Begin()
        {
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
        }

        public static void Update()
        {
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
    }

    public class ArmController
    {
        [Reveal(90, 210)]
        public double velocity = 100.5;
    }
}