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


        [UnmanagedCallersOnly]
        public static void Begin()
        {
            Console.WriteLine("Hello from .NET!");

            SayBonk();
        }

        public static void SayBonk()
        {
            Console.WriteLine("bonk");

            // if (Input.isReleased())
            // {
            //     Console.WriteLine("released up");
            // }
        }
        
       [UnmanagedCallersOnly]
        public static void Update()
        {
            Console.WriteLine("Update Script");
        }


        [UnmanagedCallersOnly]
        public static void _Input()
        {
           SayBonk();
        }
    }

    public class ArmController
    {
        [Reveal(90, 210)]
        public double velocity = 100.5;
    }
}