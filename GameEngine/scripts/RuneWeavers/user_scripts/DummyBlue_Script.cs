using System;

namespace Eno
{
    public class DummyBlue_Script : ScriptBase
    {
        private Enemy? _self;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.health         = 9999f;
                _self.maxHealth      = 9999f;
                _self.collisionGroup = 1;
            }
            EnemyColorRegistry.Register(ID, 2); // 2 = Ice / Blue
        }

        public void Update() { _self!.velocity = new Vector2(0f, 0f); }

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
