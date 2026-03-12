using System;
using System.Collections.Generic;

namespace Eno
{
	public class Runes_Script : ScriptBase
    {
        public void Begin()
        {

		}

        public void Update()
        {

		}
	
        public void Disabled_enter()
        {
            Self<Sprite2D>()?.setSize(40f, 40f);
        }

        public void Disabled_update()
        {
        }

        public void Disabled_exit()
        {
        }

        public void Ready_enter()
        {
            Self<Sprite2D>()?.setSize(56f, 56f);
        }

        public void Ready_update()
        {
        }

        public void Ready_exit()
        {
        }

        public void Activated_enter()
        {
            Self<Sprite2D>()?.setSize(56f, 56f);
        }

        public void Activated_update()
        {
        }

        public void Activated_exit()
        {
        }

        public void Nullified_enter()
        {
            Self<Sprite2D>()?.setSize(40f, 40f);
        }

        public void Nullified_update()
        {
        }

        public void Nullified_exit()
        {
        }
	


	


	}
}
			