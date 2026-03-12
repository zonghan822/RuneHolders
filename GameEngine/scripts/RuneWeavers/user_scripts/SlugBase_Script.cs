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

        private Enemy? _self;
        private Actor? _player;
        private bool   _playerSearched = false;
        private float  _pathTimer  = 0f;
        private float  _hurtTimer  = 0f;
        private float  _deathTimer = 0f;
        private float  _spawnTimer = 0f;
        private bool   _isDying    = false;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self != null)
            {
                _self.useGravity  = false;
                _self.isKinematic = false;
            }
            EnemyColorRegistry.Register(ID, EnemyColor);
        }

        public void Update() { }

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
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) <= awarenessRadius)
                _self.changeState("Walk");
        }
        public void Idle_exit() { }

        public void Walk_enter() { _pathTimer = pathRefreshInterval; }
        public void Walk_update()
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
        public void Walk_exit()
        {
            _self?.clearPath();
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }

        public void Frozen_enter() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        public void Hurt_enter()
        {
            _hurtTimer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Hurt_update()
        {
            _hurtTimer += Time.deltaTime;
            if (_hurtTimer >= hurtDuration)
            {
                if (_self?.isDead == true)
                    _self.changeState("Death");
                else
                    _self?.changeState("Walk");
            }
        }
        public void Hurt_exit() { }

        public void Death_enter()
        {
            _deathTimer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
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
