using System;

namespace Eno
{
	public class Spell_Cast_Script : ScriptBase
    {
        private const float WINDUP_DURATION  = 1.0f;
        private const float EXPLODE_DURATION = 0.6f;

        public float damage = 50f;

        private float _timer = 0f;

        public void Begin()
        {
            Self<KinematicBody2D>()?.changeState("Windup");
        }

        public void Update() { }

        // ── Windup ────────────────────────────────────────────────────────────────

        public void Windup_enter()
        {
            _timer = 0f;
            var self = Self<KinematicBody2D>();
            if (self == null) return;
            self.animation = "Windup";
            self.setAnimationLooping(false);
            uint frames = self.getAnimationFrameCount();
            if (frames > 0) self.setAnimationFrameDuration(WINDUP_DURATION / frames);
        }

        public void Windup_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= WINDUP_DURATION)
                Self<KinematicBody2D>()?.changeState("Explode");
        }

        public void Windup_exit() { }

        // ── Explode ───────────────────────────────────────────────────────────────

        public void Explode_enter()
        {
            _timer = 0f;
            var self = Self<KinematicBody2D>();
            if (self != null)
            {
                self.animation = "Explode";
                self.setAnimationLooping(false);
                uint frames = self.getAnimationFrameCount();
                if (frames > 0) self.setAnimationFrameDuration(EXPLODE_DURATION / frames);
            }
            DamageCollidingEnemies();
        }

        public void Explode_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= EXPLODE_DURATION)
                FreeNode(Self<KinematicBody2D>());
        }

        public void Explode_exit() { }

        // ── Helpers ───────────────────────────────────────────────────────────────

        private void DamageCollidingEnemies()
        {
            var self = Self<KinematicBody2D>();
            if (self == null) return;
            foreach (var enemy in GetAllEnemies())
            {
                if (self.isColliding(enemy))
                    enemy.health = MathF.Max(0f, enemy.health - damage);
            }
        }
    }
}
