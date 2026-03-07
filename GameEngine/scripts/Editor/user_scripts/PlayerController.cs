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

        public float speed = 200f;

        private static InputAction? ia2;

        public void Begin()
        {
            Console.WriteLine($"ID on script.cs: {this.ID}");

            ia2 = new InputAction("Sprint")
                .AddBinding(KeyCode.W, Mod.Shift);         // Sprints when W and left shift are pressed

            ia2.when += Sprint;

            ia2.Enable();

            Bridge.SetPlayerScriptedMovement(true);

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

            float inputX = 0f;
            float inputY = 0f;

            if (Input.IsKeyHeld(KeyCode.W)) inputY -= 1f;
            if (Input.IsKeyHeld(KeyCode.S)) inputY += 1f;
            if (Input.IsKeyHeld(KeyCode.A)) inputX -= 1f;
            if (Input.IsKeyHeld(KeyCode.D)) inputX += 1f;

            float length = (float)Math.Sqrt(inputX * inputX + inputY * inputY);
            if (length > 0.001f)
            {
                inputX /= length;
                inputY /= length;

                float dt = Bridge.GetDeltaTime();
                Bridge.GetPlayerPosition(out float px, out float py);
                Bridge.SetPlayerPosition(px + inputX * speed * dt, py + inputY * speed * dt);
            }
        }

        private static void Sprint()
        {
            Console.WriteLine("Sprinting right now!");
        }
    
		public void _when_node1_started()
        {

        }
    }
}