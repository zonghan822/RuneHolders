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

        private static CustomInput? customAct;

        [UnmanagedCallersOnly]
        public static void Begin()
        {
            // Creating a custom input
            customAct = new CustomInput(
                key: InputKey.Keycode_K,
                action: InputAction.Press,
                mod: InputMod.None);

            customAct.Enable();

            Console.WriteLine("Hello from Player controller");
        }
        
       [UnmanagedCallersOnly]
        public static void Update()
        {
            Console.WriteLine("Update Player Controller Script");
        }


        // [UnmanagedCallersOnly]
        // public static void Update()
        // {
        //     SayBonk();

        //     if (customAct != null && customAct.pressed())
        //     {
        //         Console.WriteLine("Triggered custom!");
        //     }
        // }

        // Only bonk if b is held down
        public static void SayBonk()
        {
            // Testing releasing a button
            if (Input.GetKeyReleased(InputKey.Keycode_B))
                Console.WriteLine("bonk");
            
            // Testing holding down a button
            if (Input.GetKeyHeld(InputKey.Keycode_S))
                Console.WriteLine("splash");
        
            // Testing pressing a button
            if (Input.GetKeyPressed(InputKey.Keycode_A))
                Console.WriteLine("atsuuuuuuuuuu");
        }
    }

    public class ArmController
    {
        [Reveal(90, 210)]
        public double velocity = 100.5;

        [UnmanagedCallersOnly]
        public static void Begin()
        {
            Console.WriteLine("Hello from ArmController!");
        }

               [UnmanagedCallersOnly]
        public static void Update()
        {
            Console.WriteLine("Update Arm Controller");
        }

    }
}