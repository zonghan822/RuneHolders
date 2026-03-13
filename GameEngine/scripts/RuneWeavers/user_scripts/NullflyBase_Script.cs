using System;

namespace Eno
{
    public class NullflyBase_Script : ScriptBase
    {
        public int   runeIndex       = 0;      // 0-5: which rune this Nullfly bans
        public float cloakRadius     = 150f;   // player closer than this → start cloaking
        public float uncloakRadius   = 220f;   // player farther than this → uncloak
        public float shockedDuration = 0.3f;
        public float windUpDuration  = 0.4f;
        public float hurtDuration    = 0.5f;
        public float deathDuration   = 1.0f;
        public float spawnDuration   = 0.5f;

        protected virtual int EnemyColor => 0;

        private Enemy?   _self;
        private Actor?   _player;
        private Sprite2D? _healthBar;
        private bool     _playerSearched = false;

        private const string HP_BAR_PREFAB   = "EnemyHealthBar";
        private const float  HP_BAR_OFFSET_Y = -50f;
        public float contactDamageCooldown = 1.0f;

        private float  _timer        = 0f;
        private float  _contactTimer = 0f;
        private bool   _isCloaked    = false;
        private bool   _isDying      = false;

        // ── Cloak toggle ─────────────────────────────────────────────────────────

        private void SetCloaked(bool cloaked)
        {
            if (_isCloaked == cloaked) return;
            _isCloaked = cloaked;
            if (_self != null) _self.visible = !cloaked;
            if (cloaked)
                EnemyColorRegistry.UnbanRune(runeIndex);
            else
                EnemyColorRegistry.BanRune(runeIndex);
        }

        // ── Lifecycle ────────────────────────────────────────────────────────────

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.useGravity  = false;
                _self.isKinematic = true;
                _self.velocity    = new Vector2(0f, 0f);
                _self.maxHealth   = 20f;
                _self.health      = 20f;
                _self.damage      = 15f;
            }
            EnemyColorRegistry.Register(ID, EnemyColor);
            EnemyColorRegistry.BanRune(runeIndex);   // starts visible → rune banned

            if (_self != null)
            {
                var hb = SpawnPrefab(HP_BAR_PREFAB, _self.position.x, _self.position.y + HP_BAR_OFFSET_Y, _self);
                _healthBar = hb as Sprite2D;
            }
        }

        public void Update()
        {
            if (_isDying || _self == null) return;

            if (_healthBar != null)
            {
                _healthBar.position = new Vector2(_self.position.x, _self.position.y + HP_BAR_OFFSET_Y);
                float pct = _self.maxHealth > 0f ? _self.health / _self.maxHealth : 0f;
                _healthBar.setUV(new Vector2(0f, 0f), new Vector2(pct, 1f));
            }

            if (_isCloaked) return;
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

        // ── Spawn ────────────────────────────────────────────────────────────────

        public void Spawn_enter() { _timer = 0f; }
        public void Spawn_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= spawnDuration)
                _self?.changeState("Idle");
        }
        public void Spawn_exit() { }

        // ── Idle (visible, rune banned) ──────────────────────────────────────────

        public void Idle_enter()  { SetCloaked(false); }
        public void Idle_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) <= cloakRadius)
                _self.changeState("Shocked");
        }
        public void Idle_exit() { }

        // ── Shocked (brief reaction before cloaking) ─────────────────────────────

        public void Shocked_enter() { _timer = 0f; }
        public void Shocked_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= shockedDuration)
                _self?.changeState("WindUp");
        }
        public void Shocked_exit() { }

        // ── WindUp (transition into cloak) ───────────────────────────────────────

        public void WindUp_enter() { _timer = 0f; }
        public void WindUp_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= windUpDuration)
                _self?.changeState("Channel");
        }
        public void WindUp_exit() { }

        // ── Channel (invisible, rune unbanned) ───────────────────────────────────

        public void Channel_enter() { SetCloaked(true); }
        public void Channel_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) > uncloakRadius)
                _self.changeState("Idle");
        }
        public void Channel_exit() { SetCloaked(false); }

        // ── Frozen ───────────────────────────────────────────────────────────────

        public void Frozen_enter() { }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        // ── Hurt ─────────────────────────────────────────────────────────────────

        public void Hurt_enter() { _timer = 0f; }
        public void Hurt_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= hurtDuration)
            {
                if (_self?.isDead == true)
                    _self.changeState("Death");
                else
                    _self?.changeState("Idle");
            }
        }
        public void Hurt_exit() { }

        // ── Death ────────────────────────────────────────────────────────────────

        public void Death_enter()
        {
            _timer = 0f;
            // If dying while cloaked, clean up ban state without re-banning
            if (_isCloaked)
            {
                _isCloaked = false;
                if (_self != null) _self.visible = true;
                EnemyColorRegistry.UnbanRune(runeIndex);
            }
            else
            {
                EnemyColorRegistry.UnbanRune(runeIndex);
            }
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

        // ── Helpers ──────────────────────────────────────────────────────────────

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
    }
}
