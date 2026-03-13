using System;

namespace Eno
{
    public class BatBase_Script : ScriptBase
    {
        public float chaseSpeed      = 120f;
        public float dashSpeed       = 240f;
        public float hurtDuration    = 0.3f;
        public float deathDuration   = 1.0f;
        public float spawnDuration   = 0.5f;
        public float contactDamageCooldown = 1.0f;

        // 1 unit = 55 pixels
        private const float TRIGGER_RADIUS      = 165f;
        private const float CHARGE_DISTANCE     = 330f;
        private const float WINDUP_DURATION     = 0.75f;
        private const float TRACKING_DURATION   = 0.5f;  // first 0.5s tracks player

        protected virtual int EnemyColor => 0;

        private Enemy?  _self;
        private Actor?  _player;
        private bool    _playerSearched = false;
        private float   _timer          = 0f;
        private float   _contactTimer   = 0f;
        private Vector2 _dashDir        = new Vector2(1f, 0f);
        private Vector2 _prevPos;
        private float   _chargedPixels  = 0f;
        private bool    _isDying        = false;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.useGravity  = false;
                _self.isKinematic = false;
                _self.maxHealth   = 10f;
                _self.health      = 10f;
                _self.damage      = 20f;
            }
            EnemyColorRegistry.Register(ID, EnemyColor);
        }

        public void Update()
        {
            if (_isDying || _self == null) return;
            FindPlayer();
            if (_player == null) return;

            if (_contactTimer > 0f)
            {
                _contactTimer -= Time.deltaTime;
                return;
            }

            if (_self.isColliding(_player))
            {
                _player.health = MathF.Max(0f, _player.health - _self.damage);
                _player.changeState("Hurt");
                _contactTimer = contactDamageCooldown;
            }
        }

        // ── Spawn ─────────────────────────────────────────────────────────────────

        public void Spawn_enter() { _timer = 0f; }
        public void Spawn_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= spawnDuration)
                _self?.changeState("Idle");
        }
        public void Spawn_exit() { }

        // ── Idle ──────────────────────────────────────────────────────────────────

        public void Idle_enter() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }
        public void Idle_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) <= TRIGGER_RADIUS)
                _self.changeState("WindUp");
        }
        public void Idle_exit() { }

        // ── WindUp ────────────────────────────────────────────────────────────────

        public void WindUp_enter() { _timer = 0f; }
        public void WindUp_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;

            _timer += Time.deltaTime;

            // Tracking phase: first TRACKING_DURATION seconds face the player
            if (_timer <= TRACKING_DURATION)
            {
                float dx = _player.position.x - _self.position.x;
                float dy = _player.position.y - _self.position.y;
                _dashDir = Norm(dx, dy);

                if (_dashDir.x != 0f)
                {
                    Vector2 s = _self.scale;
                    _self.scale = new Vector2(_dashDir.x > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
                }
            }

            if (_timer >= WINDUP_DURATION)
                _self.changeState("Attack");
        }
        public void WindUp_exit() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }

        // ── Attack (charge) ───────────────────────────────────────────────────────

        public void Attack_enter()
        {
            _timer         = 0f;
            _chargedPixels = 0f;
            if (_self != null)
            {
                _prevPos       = _self.position;
                _self.velocity = new Vector2(_dashDir.x * dashSpeed, _dashDir.y * dashSpeed);
            }
        }
        public void Attack_update()
        {
            if (_self == null) return;

            Vector2 curPos = _self.position;
            float moved    = Dist(curPos, _prevPos);
            _chargedPixels += moved;
            _prevPos        = curPos;

            // Wall hit: expected to move but barely did
            float expectedMove = dashSpeed * Time.deltaTime;
            bool  hitWall      = moved < expectedMove * 0.15f;

            if (_chargedPixels >= CHARGE_DISTANCE || hitWall)
            {
                _self.velocity = new Vector2(0f, 0f);
                _self.changeState("Idle");
            }
        }
        public void Attack_exit() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }

        // ── Frozen ────────────────────────────────────────────────────────────────

        public void Frozen_enter() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        // ── Hurt ──────────────────────────────────────────────────────────────────

        public void Hurt_enter()
        {
            _timer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Hurt_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= hurtDuration)
            {
                if (_self?.isDead == true)
                    _self.changeState("Death");
                else
                    _self?.changeState("WindUp");
            }
        }
        public void Hurt_exit() { }

        // ── Death ─────────────────────────────────────────────────────────────────

        public void Death_enter()
        {
            _timer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Death_update()
        {
            if (_isDying) return;
            _timer += Time.deltaTime;
            if (_timer >= deathDuration)
            {
                _isDying = true;
                EnemyColorRegistry.Unregister(ID);
                FreeNode(_self!);
            }
        }
        public void Death_exit() { }

        // ── Helpers ───────────────────────────────────────────────────────────────

        private void FindPlayer()
        {
            if (_playerSearched) return;
            _playerSearched = true;
            try { _player = GetNode<Actor>("Player"); }
            catch { _player = null; }
        }

        private static float Dist(Vector2 a, Vector2 b)
        {
            float dx = b.x - a.x, dy = b.y - a.y;
            return MathF.Sqrt(dx * dx + dy * dy);
        }

        private static Vector2 Norm(float dx, float dy)
        {
            float len = MathF.Sqrt(dx * dx + dy * dy);
            if (len < 0.001f) return new Vector2(1f, 0f);
            return new Vector2(dx / len, dy / len);
        }
    }
}
