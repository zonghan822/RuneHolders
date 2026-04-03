using System;
using System.Collections.Generic;

namespace Eno
{
    public class DummyYellow_Script : ScriptBase
    {
        public static bool WasHit = false;
        private static List<DummyYellow_Script> _instances = new List<DummyYellow_Script>();

        private const float HURT_DURATION = 0.3f;

        private Enemy? _self;
        private float _timer = 0f;

        public void Begin()
        {
            _instances.Add(this);
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.health         = 9999f;
                _self.maxHealth      = 9999f;
                _self.collisionGroup = 1;
            }
            EnemyColorRegistry.Register(ID, 1); // 1 = Light / Yellow
        }

        public void Update() { _self!.velocity = new Vector2(0f, 0f); }

        public void Idle_enter() { }
        public void Idle_update() { }
        public void Idle_exit() { }

        public void Hurt_enter()
        {
            WasHit = true;
            _timer = 0f;
            if (_self != null)
            {
                _self.setAnimationLooping(false);
                uint frames = _self.getAnimationFrameCount();
                if (frames > 0) _self.setAnimationFrameDuration(HURT_DURATION / frames);
            }
        }
        public void Hurt_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= HURT_DURATION)
                _self?.changeState("Idle");
        }
        public void Hurt_exit() { }

        public static void Cleanup()
        {
            WasHit = false;
            foreach (var inst in _instances)
            {
                EnemyColorRegistry.Unregister(inst.ID);
                if (inst._self != null) inst.FreeNode(inst._self);
            }
            _instances.Clear();
        }
    }
}
