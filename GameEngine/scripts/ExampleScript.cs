using System;
using System.Runtime.InteropServices;

using Eno;

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

        public enum Rarity
        {
            common,
            rare,
            epic,
            legendary
        }

        [Reveal()]
        public Rarity r = Rarity.common;

        public void Begin()
        {
            //----------------------------------------
            // Testing adding and removing children
            Node2D enemy = CreateNode<Node2D>();

            // Adding a child
            if (this.ID == 3)
            {
                Console.WriteLine("Adding child");
                AddChild(enemy);
            }
            
            // Removing a child
            Node2D? node = GetNode<Node2D>("Node3");
            if (this.ID == 1)
            {
                Console.WriteLine("Removing child");
                Unparent(node);
            }

            // FreeNode<Node2D>(GetNode<Node2D>("Node3"));

            //----------------------------------------
            // Get Child count
            Console.WriteLine($"Current Node: {ID}");
            Console.WriteLine($"Children count: {GetChildCount()}");

            // Get parent
            if (this.ID == 3)
                Console.WriteLine("Currently on node 3's script");
            
            var parent = GetParentNode<Node2D>();
            if (parent != null)
                Console.WriteLine($"Parent id: {parent.ID}");
            //----------------------------------------

            // Do not uncomment this while the "Testing adding and removing children"
            // section is active as there will be an error! Purely for testing purposes
            //
            // // Testing getting a node/child
            // // ----------------------------
            // // Testing GetNode
            // uint node3_z_index = GetNode<Node2D>("Node3").z_index;
            // Console.WriteLine($"1 Node3's z index: {node3_z_index}");

            // if (this.ID == 1)
            //     GetChild<Node2D>(0).z_index = 860;

            // if (this.ID == 3)
            //     Self<Node2D>().z_index = 720;

            // node3_z_index = GetNode<Node2D>("Node3").z_index;
            // Console.WriteLine($"2 Node3's z index: {node3_z_index}");
            // // ----------------------------

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
            // Triggers when pressing B
            if (Bridge.IsKeyPressed(KeyCode.Num1))
            {
                Console.WriteLine("Pressed 1!");
            }

            // Triggers when holding C
            if (Bridge.IsKeyHeld(KeyCode.C))
            {
                Console.WriteLine("Held C!");
            }

            // Testing if the right entity is affected
            // by changing the audio volume the entity is on
            if (Bridge.IsKeyPressed(KeyCode.V))
            {
                Console.WriteLine("Pressed V, volume should be modified on entity 1 now!");
                //ModifyVolume(5);
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

		public void _when_node3_started()
        {
            Console.WriteLine("3 Node1 has started with audio playing! enemycon.cs");
        }
    }

    // public class ArmController : ScriptBase
    // {
    //     [Reveal(90, 210)]
    //     public double velocity = 100.5;
    // }
}