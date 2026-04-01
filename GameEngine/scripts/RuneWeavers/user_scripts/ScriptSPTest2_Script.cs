using System;

namespace Eno
{
    // Attach to the RIGHT Actor entity.
    // Press = to fire a spatial play from this actor's world position.
    public class ScriptSPTest2_Script : ScriptBase
    {
        private AudioNode? _audio;
        private bool _wasEqualPressed = false;

        public void Begin()
        {
            var self = Self<Actor>();
            if (self == null) { Console.WriteLine("[SPTest2] Begin: Self<Actor> is NULL"); return; }

            _audio = SpawnPrefab("DefaultAudio", self.position.x, self.position.y, self) as AudioNode;
            if (_audio == null) { Console.WriteLine("[SPTest2] Begin: SpawnPrefab DefaultAudio failed"); return; }

            _audio.sound        = "Spellbook_Close_01";
            _audio.spatialAudio = true;
            Console.WriteLine("[SPTest2] Begin: audio node ready");
        }

        public void Update()
        {
            bool equalDown = Input.isKeyPressed(KeyCode.Equal);

            if (equalDown && !_wasEqualPressed)
            {
                if (_audio == null) { Console.WriteLine("[SPTest2] audio is null"); return; }

                var self = Self<Actor>();
                float x = self != null ? self.position.x : Bridge.GetPositionX(ID);
                float y = self != null ? self.position.y : Bridge.GetPositionY(ID);

                _audio.stop();
                Console.WriteLine($"[SPTest2] spatialPlay at ({x}, {y})");
                _audio.spatialPlay(_audio.ID, x, y);
            }

            _wasEqualPressed = equalDown;
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
