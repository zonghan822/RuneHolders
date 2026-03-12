using System;
using System.Collections.Generic;

namespace Eno
{
    // Attach this script to an entity that has an AudioComponent.
    // Press E to play the audio, Q to stop it.
    public class AudioTest_Script : ScriptBase
    {
        private bool _wasEPressed = false;
        private bool _wasQPressed = false;

        public void Begin()
        {
        }

        public void Update()
        {
            bool eDown = Bridge.IsKeyPressed(KeyCode.E);
            bool qDown = Bridge.IsKeyPressed(KeyCode.Q);

            // Play on the first frame the key goes down (not every frame it's held)
            if (eDown && !_wasEPressed)
                Bridge.AudioPlay(ID);

            // Stop on the first frame the key goes down
            if (qDown && !_wasQPressed)
                Bridge.AudioStop(ID);

            _wasEPressed = eDown;
            _wasQPressed = qDown;
        }
    
        public void Default_enter()
        {
            // TODO: Add enter logic for Default state
        }

        public void Default_update()
        {
            // TODO: Add update logic for Default state
        }

        public void Default_exit()
        {
            // TODO: Add exit logic for Default state
        }
    }
}
