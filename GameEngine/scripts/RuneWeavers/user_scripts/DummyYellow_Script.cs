using System;

namespace Eno
{
    public class DummyYellow_Script : ScriptBase
    {
        public static bool WasHit = false;

        private Enemy? _self;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.health    = 9999f;
                _self.maxHealth = 9999f;
            }
            EnemyColorRegistry.Register(ID, 1); // 1 = Light / Yellow
        }

        public void Update() { _self.velocity = 0.0f; }

        public void Idle_enter() { }
        public void Idle_update() { }
        public void Idle_exit() { }

        public void Hurt_enter()
        {
            EnemyColorRegistry.Unregister(ID);
            FreeNode(_self!);
        }
        public void Hurt_update() { }
        public void Hurt_exit() { }
    }
}
