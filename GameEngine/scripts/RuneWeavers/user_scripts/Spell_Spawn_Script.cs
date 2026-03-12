using System;
using System.Collections.Generic;

namespace Eno
{
	public class Spell_Spawn_Script : ScriptBase
    {
        private const float uvW = 1.0f / 4.0f;

        public void Begin()
        {

		}

        public void Update()
        {

		}

        public void Cast1_enter()
        {
            Self<Sprite2D>()?.setUV(new Vector2(0 * uvW, 0f), new Vector2(1 * uvW, 1f));
        }

        public void Cast1_update()
        {
        }

        public void Cast1_exit()
        {
        }

        public void Cast2_enter()
        {
            Self<Sprite2D>()?.setUV(new Vector2(1 * uvW, 0f), new Vector2(2 * uvW, 1f));
        }

        public void Cast2_update()
        {
        }

        public void Cast2_exit()
        {
        }

        public void Cast3_enter()
        {
            Self<Sprite2D>()?.setUV(new Vector2(2 * uvW, 0f), new Vector2(3 * uvW, 1f));
        }

        public void Cast3_update()
        {
        }

        public void Cast3_exit()
        {
        }

        public void Cast4_enter()
        {
            Self<Sprite2D>()?.setUV(new Vector2(3 * uvW, 0f), new Vector2(4 * uvW, 1f));
        }

        public void Cast4_update()
        {
        }

        public void Cast4_exit()
        {
        }
	


	}
}
			