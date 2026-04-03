using System;

namespace Eno
{
    public class DummyRed_Script : ScriptBase
    {
        public static bool WasHit = false;
        private static DummyRed_Script? _instance;

        private const float HURT_DURATION = 0.3f;

        private Enemy? _self;
        private float _timer = 0f;

        public void Begin()
        {
            _instance = this;
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.health         = 9999f;
                _self.maxHealth      = 9999f;
                _self.collisionGroup = 1;
            }
            EnemyColorRegistry.Register(ID, 0); // 0 = Fire / Red
        }

        public void Update()
        {
            if (_self == null) return;
            _self.velocity = new Vector2(0f, 0f);
            if (_self.health <= 0f)
            {
                EnemyColorRegistry.Unregister(ID);
                _instance = null;
                FreeNode(_self);
            }
        }

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
            if (_instance == null) return;
            EnemyColorRegistry.Unregister(_instance.ID);
            if (_instance._self != null) _instance.FreeNode(_instance._self);
            _instance = null;
        }
    }
}
