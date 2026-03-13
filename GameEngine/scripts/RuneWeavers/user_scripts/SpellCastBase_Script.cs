using System;

namespace Eno
{
    public class SpellCastBase_Script : ScriptBase
    {
        private const float WINDUP_DURATION  = 1.0f;
        private const float EXPLODE_DURATION = 0.6f;

        // Subclasses hardcode this: 0 = Fire/Red, 1 = Light/Yellow, 2 = Ice/Blue
        protected virtual int SpellType => 0;

        // Audio node names indexed by SpellType: 0=Red, 1=Yellow, 2=Blue
        private static readonly string[] SpellSoundNames = new string[]
        {
            "SpellCast_02",   // SpellType 0 – Fire / Red
            "SpellCast_03",   // SpellType 1 – Light / Yellow
            "SpellCast_01",   // SpellType 2 – Ice / Blue
        };

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

            if (SpellType >= 0 && SpellType < SpellSoundNames.Length)
            {
                try { GetNode<AudioNode>(SpellSoundNames[SpellType])?.play(); } catch { }
            }
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
            DamageMatchingEnemies();
        }

        public void Explode_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= EXPLODE_DURATION)
                FreeNode(Self<KinematicBody2D>());
        }

        public void Explode_exit() { }

        // ── Helpers ───────────────────────────────────────────────────────────────

        private void DamageMatchingEnemies()
        {
            var self = Self<KinematicBody2D>();
            if (self == null) return;

            Actor? player = GetNode<Actor>("Player");
            float damage = player != null ? player.damage : 0f;

            foreach (Enemy enemy in GetAllEnemies())
            {
                if (!self.isColliding(enemy)) continue;

                if (!EnemyColorRegistry.Colors.TryGetValue(enemy.ID, out int color)) continue;
                if (color != SpellType) continue;

                enemy.health = MathF.Max(0f, enemy.health - damage);
                if (enemy.isDead)
                    enemy.changeState("Death");
                else
                    enemy.changeState("Hurt");
            }
        }
    }
}
