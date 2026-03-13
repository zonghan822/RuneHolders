using System;

namespace Eno
{
    public class SlugBase_Script : ScriptBase
    {
        public float chaseSpeed          = 60f;
        public float awarenessRadius     = 250f;
        public float pathRefreshInterval = 0.5f;
        public float hurtDuration        = 0.4f;
        public float deathDuration       = 1.0f;
        public float spawnDuration       = 0.5f;

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

        private float  _pathTimer       = 0f;
        private float  _hurtTimer       = 0f;
        private float  _deathTimer      = 0f;
        private float  _spawnTimer      = 0f;
        private float  _contactTimer    = 0f;
        private bool   _isDying         = false;
        private bool   _deathTriggered  = false;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.useGravity  = false;
                _self.isKinematic = false;
                _self.maxHealth   = 30f;
                _self.health      = 30f;
                _self.damage      = 10f;
            }
            EnemyColorRegistry.Register(ID, EnemyColor);

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
            FindPlayer();
            if (_player == null) return;

            if (_contactTimer > 0f)
            {
                _contactTimer -= Time.deltaTime;
                return;
            }

            if (_self.isColliding(_player))
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

        public void Spawn_enter() { _spawnTimer = 0f; }
        public void Spawn_update()
        {
            _spawnTimer += Time.deltaTime;
            if (_spawnTimer >= spawnDuration)
                _self?.changeState("Idle");
        }
        public void Spawn_exit() { }

        public void Idle_enter()
        {
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Idle_update()
        {
            _self?.changeState("Chase");
        }
        public void Idle_exit() { }

        public void Chase_enter()
        {
            _pathTimer = pathRefreshInterval;
            try { Bridge.AudioPlayInstantLoop("SFX", "Trailblight_Move_Loop"); } catch { }
        }
        public void Chase_update()
        {
            FindPlayer();
            if (_self == null || _player == null) return;

            _pathTimer += Time.deltaTime;
            if (_pathTimer >= pathRefreshInterval)
            {
                _pathTimer = 0f;
                _self.computePath(_player.position);
            }

            if (_self.hasPath)
            {
                Vector2 wp = _self.nextWaypoint;
                float dx = wp.x - _self.position.x;
                float dy = wp.y - _self.position.y;

                if (dx * dx + dy * dy < 32f * 32f)
                {
                    _self.advance();
                }
                else
                {
                    Vector2 dir = Norm(dx, dy);
                    _self.velocity = new Vector2(dir.x * chaseSpeed, dir.y * chaseSpeed);
                    if (dir.x != 0f)
                    {
                        Vector2 s = _self.scale;
                        _self.scale = new Vector2(dir.x > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
                    }
                }
            }
            else
            {
                float dx = _player.position.x - _self.position.x;
                float dy = _player.position.y - _self.position.y;
                float distSq = dx * dx + dy * dy;
                if (distSq > 5f * 5f)
                {
                    Vector2 dir = Norm(dx, dy);
                    _self.velocity = new Vector2(dir.x * chaseSpeed, dir.y * chaseSpeed);
                    if (dir.x != 0f)
                    {
                        Vector2 s = _self.scale;
                        _self.scale = new Vector2(dir.x > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
                    }
                }
                else
                {
                    _self.velocity = new Vector2(0f, 0f);
                }
            }
        }
        public void Chase_exit()
        {
            _self?.clearPath();
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
            try { Bridge.AudioStopInstant("Trailblight_Move_Loop"); } catch { }
        }

        public void Frozen_enter()
        {
            if (_deathTriggered) { _self?.changeState("Death"); return; }
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        public void Hurt_enter()
        {
            _hurtTimer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
            try { Bridge.AudioPlayInstant("SFX", "Trailblight_Hurt_02"); } catch { }
        }
        public void Hurt_update()
        {
            _hurtTimer += Time.deltaTime;
            if (_hurtTimer >= hurtDuration)
            {
                if (_self?.isDead == true)
                    _self.changeState("Death");
                else
                    _self?.changeState("Chase");
            }
        }
        public void Hurt_exit() { }

        public void Death_enter()
        {
            _deathTimer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
            try { Bridge.AudioPlayInstant("SFX", "Trailblight_Death_02"); } catch { }
        }
        public void Death_update()
        {
            if (_isDying) return;
            _deathTimer += Time.deltaTime;
            if (_deathTimer >= deathDuration)
            {
                _isDying = true;
                EnemyColorRegistry.Unregister(ID);
                FreeNode(_self!);
            }
        }
        public void Death_exit() { }

        private void FindPlayer()
        {
            if (_playerSearched) return;
            _playerSearched = true;
            try { _player = GetNode<Actor>("Player"); }
            catch { _player = null; }
        }

        protected static float Dist(Vector2 a, Vector2 b)
        {
            float dx = b.x - a.x, dy = b.y - a.y;
            return MathF.Sqrt(dx * dx + dy * dy);
        }

        protected static Vector2 Norm(float dx, float dy)
        {
            float len = MathF.Sqrt(dx * dx + dy * dy);
            if (len < 0.001f) return new Vector2(0f, 0f);
            return new Vector2(dx / len, dy / len);
        }
    }
}
