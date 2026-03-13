using System;

namespace Eno
{
    public class NullflyBase_Script : ScriptBase
    {
        public int   runeIndex       = 5;      // 0-5: which rune this Nullfly bans
        public float cloakRadius     = 220f;   // player closer than this → cloak
        public float uncloakRadius   = 300f;   // player farther than this → uncloak
        public float idleDuration    = 0.5f;     // time in Idle before channelling
        public float shockedDuration = 0.2f;
        public float windUpDuration  = 0.3f;
        public float hurtDuration    = 0.5f;
        public float deathDuration   = 1.0f;
        public float spawnDuration   = 0.5f;

        protected virtual int EnemyColor => 0;

        private Enemy?    _self;
        private Actor?    _player;
        private Sprite2D? _hpBarEmpty;
        private Sprite2D? _hpBarFull;
        private float     _hpBarFullInitialScaleX = 1f;
        private float     _hpBarFullHalfWidth     = 0f;
        private bool      _playerSearched = false;

        private const string HP_BAR_EMPTY_PREFAB = "EnemyHealthBar_Empty";
        private const string HP_BAR_FULL_PREFAB  = "EnemyHealthBar_Full";
        private const float  HP_BAR_OFFSET_Y     = -50f;
        public float contactDamageCooldown = 1.0f;

        private float  _timer          = 0f;
        private float  _contactTimer  = 0f;
        private bool   _isCloaked     = false;
        private bool   _isDying       = false;
        private bool   _deathTriggered = false;

        // ── Cloak toggle ─────────────────────────────────────────────────────────

        private void SetCloaked(bool cloaked)
        {
            if (_isCloaked == cloaked) return;
            _isCloaked = cloaked;
            if (_self != null) _self.visible = !cloaked;
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
            // Rune starts unbanned; only Channel state bans it

            if (_self != null)
            {
                float hx = _self.position.x, hy = _self.position.y + HP_BAR_OFFSET_Y;
                _hpBarEmpty = SpawnPrefab(HP_BAR_EMPTY_PREFAB, hx, hy, _self) as Sprite2D;
                _hpBarFull  = SpawnPrefab(HP_BAR_FULL_PREFAB,  hx, hy, _self) as Sprite2D;
                if (_hpBarFull != null)
                {
                    _hpBarFullInitialScaleX = _hpBarFull.scale.x;
                    _hpBarFullHalfWidth     = _hpBarFull.width * 0.5f;
                }
            }
        }

        public void Update()
        {
            if (_isDying || _self == null) return;

            // Force death if health reached zero outside of Hurt state
            if (!_deathTriggered && _self.health <= 0f)
            {
                _deathTriggered = true;
                _self.changeState("Death");
                return;
            }

            if (_hpBarEmpty != null || _hpBarFull != null)
            {
                float hx  = _self.position.x;
                float hy  = _self.position.y + HP_BAR_OFFSET_Y;
                float pct = _self.maxHealth > 0f ? _self.health / _self.maxHealth : 0f;

                if (_hpBarEmpty != null) _hpBarEmpty.position = new Vector2(hx, hy);
                if (_hpBarFull  != null)
                {
                    _hpBarFull.scale    = new Vector2(_hpBarFullInitialScaleX * pct, _hpBarFull.scale.y);
                    _hpBarFull.position = new Vector2(hx + _hpBarFullHalfWidth * (pct - 1f), hy);
                }
            }

            if (_contactTimer > 0f)
            {
                _contactTimer -= Time.deltaTime;
                return;
            }

            FindPlayer();
            if (_player != null && _self.isColliding(_player))
            {
                _self.velocity = new Vector2(0f, 0f);
                _player.health = MathF.Max(0f, _player.health - _self.damage);
                if (_self.health > 0){
                    _player.changeState("Hurt");
                    _self.changeState("Idle");
                } 
                _contactTimer = contactDamageCooldown;
            }
        }

        // ── Spawn ────────────────────────────────────────────────────────────────

        public void Spawn_enter() { 
            _timer = 0f;     
        }

        public void Spawn_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= spawnDuration)
                _self?.changeState("Idle");
        }
        public void Spawn_exit() { }

        // ── Idle (visible, rune banned, timed → Channel) ─────────────────────────

        public void Idle_enter()
        {
            _timer = 0f;
            SetCloaked(false);
            EnemyColorRegistry.UnbanRune(runeIndex); // ensure rune is clean on Idle
        }
        public void Idle_update()
        {
            _timer += Time.deltaTime;
            if (_timer >= idleDuration)
                _self?.changeState("Channel");
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

        // ── WindUp (cloak + unban, then monitor distance → Idle) ────────────────

        public void WindUp_enter()
        {
            _timer = 0f;
            try { Bridge.AudioPlayInstant("SFX", "Nullfly_Attack_01"); } catch { }
        }
        public void WindUp_update()
        {
            if (!_isCloaked)
            {
                _timer += Time.deltaTime;
                if (_timer >= windUpDuration)
                {
                    SetCloaked(true);
                    EnemyColorRegistry.UnbanRune(runeIndex);
                }
            }
            else
            {
                FindPlayer();
                if (_self == null || _player == null) return;
                if (Dist(_self.position, _player.position) > uncloakRadius)
                    _self?.changeState("Idle");
            }
        }
        public void WindUp_exit()
        {
            // If hurt mid-windup before cloaking, rune was still banned by Channel — unban it
            if (!_isCloaked)
                EnemyColorRegistry.UnbanRune(runeIndex);
        }

        // ── Channel (visible, always bans rune; player near → Shocked) ───────────

        public void Channel_enter() { EnemyColorRegistry.BanRune(runeIndex); }
        public void Channel_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) <= cloakRadius)
                _self.changeState("Shocked");
        }
        public void Channel_exit() { }

        // ── Frozen ───────────────────────────────────────────────────────────────

        public void Frozen_enter()
        {
            if (_deathTriggered) { _self?.changeState("Death"); return; }
        }
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
            if (_isCloaked)
            {
                _isCloaked = false;
                if (_self != null) _self.visible = true;
                // rune already unbanned when cloaking happened
            }
            else
            {
                EnemyColorRegistry.UnbanRune(runeIndex); // may have died in Channel (banned)
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
