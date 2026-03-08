using System;
using System.Collections.Generic;

namespace Eno
{
	public class MainMenu_Background_Script : ScriptBase
    {
        private float _timer = 0f;
        private const float SwapInterval = 1f;

        public void Begin()
        {
            Bridge.ChangeState(ID, "art1");
        }

        public void Update()
        {
        }

        // Called when entering the art1 state
        public void art1_enter()
        {
            _timer = 0f;
        }

        // Called every frame while in the art1 state
        public void art1_update()
        {
            Console.WriteLine("Art1");
            _timer += Bridge.GetDeltaTime();
            if (_timer >= SwapInterval)
                Bridge.ChangeState(ID, "art2");
        }

        // Called when exiting the art1 state
        public void art1_exit()
        {
        }

        // Called when entering the art2 state
        public void art2_enter()
        {
            _timer = 0f;
        }

        // Called every frame while in the art2 state
        public void art2_update()
        {
            Console.WriteLine("Art2");
            _timer += Bridge.GetDeltaTime();
            if (_timer >= SwapInterval)
                Bridge.ChangeState(ID, "art1");
        }

        // Called when exiting the art2 state
        public void art2_exit()
        {
        }
	}
}
			