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
            Self<Sprite2D>()?.changeState("art1");
            // Play the BGM once; the audio component is configured to loop.
            Bridge.AudioPlay(ID);
        }

        public void Update()
        {
            if (Input.isMousePressed(0))
            {
                Bridge.AudioStop(ID);
                SceneManager.changeScene("Level.txt");
            }
        }

        // Called when entering the art1 state
        public void art1_enter()
        {
            _timer = 0f;
        }

        // Called every frame while in the art1 state
        public void art1_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= SwapInterval)
                Self<Sprite2D>()?.changeState("art2");
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
            _timer += Time.deltaTime;
            if (_timer >= SwapInterval)
                Self<Sprite2D>()?.changeState("art1");
        }

        // Called when exiting the art2 state
        public void art2_exit()
        {
        }
	}
}
			