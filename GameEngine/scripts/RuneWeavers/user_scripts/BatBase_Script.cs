using System;
using System.Collections.Generic;

namespace Eno
{
    public class BatBase_Script : ScriptBase
    {
        private static List<BatBase_Script> _allInstances = new List<BatBase_Script>();
        private static bool _gameEnded = false;

        public static void StopAllAudio()
        {
            _gameEnded = true;
            foreach (var inst in _allInstances)
                inst._audio?.stop();
        }

        public float chaseSpeed      = 120f;
        public float dashSpeed       = 240f;
        public float hurtDuration    = 0.3f;
        public float deathDuration   = 1.0f;
        public float prespawnDuration = 1.0f;
        public float spawnDuration   = 0.5f;
        public float pathRefreshInterval = 0.5f;
        public float contactDamageCooldown = 1.0f;
        public float dropChance = 0.3f;    // Probability [0-1] of dropping a HealthPotion on death

        private const float TRIGGER_RADIUS      = 165f;
        private const float CHARGE_DISTANCE     = 330f;
        private const float WINDUP_DURATION     = 0.75f;
        private const float TRACKING_DURATION   = 0.5f;  // first 0.5s tracks player

        protected virtual int EnemyColor => 0;

        private Enemy?    _self;
        private Actor?    _player;
        private AudioNode? _audio;
        private Sprite2D? _hpBarEmpty;
        private Sprite2D? _hpBarFull;
        private float     _hpBarFullInitialScaleX = 1f;
        private float     _hpBarFullHalfWidth     = 0f;
        private bool      _playerSearched = false;

        private const string HP_BAR_EMPTY_PREFAB = "EnemyHealthBar_Empty";
        private const string HP_BAR_FULL_PREFAB  = "EnemyHealthBar_Full";
        private const float  HP_BAR_OFFSET_Y     = -50f;

        // Audio sounds
        private const string ATTACK_SOUND = "Bloodwing_Attack_01";
        private const string WINDUP_SOUND = "Bloodwing_Windup_01";
        private const string HURT_SOUND = "Bloodwing_Hurt_02";
        private const string DEATH_SOUND = "Bloodwing_Death_01";

        private const string AUDIO               = "DefaultAudio";
        private const string HEALTH_POTION_PREFAB = "HealthPotion";

        private static readonly Random _rng = new Random();

        private float   _timer          = 0f;
        private bool    _prespawnSetup  = false;
        private bool    _spawnSetup     = false;
        private float   _pathTimer      = 0f;
        private float   _contactTimer   = 0f;
        private float   _stuckTimer     = 0f;
        private Vector2 _lastStuckPos   = new Vector2(0f, 0f);
        private Vector2 _dashDir        = new Vector2(1f, 0f);
        private Vector2 _prevPos;
        private float   _chargedPixels  = 0f;
        private bool    _dashReady      = false;

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self == null) return;
            
            // Initialize Bat Values
            _self.useGravity    = false;
            _self.isKinematic   = false;
            _self.maxHealth     = 1f;
            _self.health        = 1f;
            _self.damage        = 1f;
            _self.pathClearance  = 1;
            _self.collisionGroup = 1;
            EnemyColorRegistry.Register(ID, EnemyColor);

            // Spawn HP Bar
            float hx = _self.position.x, hy = _self.position.y + HP_BAR_OFFSET_Y;
            _hpBarEmpty = SpawnPrefab(HP_BAR_EMPTY_PREFAB, hx, hy, _self) as Sprite2D;
            _hpBarFull  = SpawnPrefab(HP_BAR_FULL_PREFAB,  hx, hy, _self) as Sprite2D;
            if (_hpBarFull != null)
            {
                _hpBarFullInitialScaleX = _hpBarFull.scale.x;
                _hpBarFullHalfWidth     = _hpBarFull.width * 0.5f;
            }

            // Audio node initialization
            _audio = SpawnPrefab(AUDIO, _self.position.x, _self.position.y, _self) as AudioNode;
            _allInstances.Add(this);
        }

        public void Update()
        {
            if (PlayerController.IsGamePaused)             
            {
                if (_self != null) _self.velocity = new Vector2(0f, 0f);
                return;
            }
            if (_self == null) return;

            // Force death if health reached zero outside of Hurt state
            if (_self.health <= 0f)
            {
                _self.changeState("Death");
                return;
            }

            // Update Healthbar
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

            // Enemy - Player Collision
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
                _player.changeState("Hurt");
                _self.changeState("Idle");
                _contactTimer = contactDamageCooldown;
            }
        }

        // ── Prespawn ──────────────────────────────────────────────────────────────
        public void Prespawn_enter()
        {
            _timer = 0f;
            _prespawnSetup = false;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Prespawn_update()
        {
            if (PlayerController.IsGamePaused) return;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
            if (!_prespawnSetup && _self != null)
            {
                uint frames = _self.getAnimationFrameCount();
                if (frames > 0)
                {
                    _prespawnSetup = true;
                    _self.setAnimationFrameDuration(prespawnDuration / frames);
                }
            }
            _timer += Time.deltaTime;
            if (_timer >= prespawnDuration)
                _self?.changeState("Spawn");
        }
        public void Prespawn_exit() { }

        // ── Spawn ─────────────────────────────────────────────────────────────────
        public void Spawn_enter()
        {
            _timer = 0f;
            _spawnSetup = false;
        }
        public void Spawn_update()
        {
            if (PlayerController.IsGamePaused) return;
            if (!_spawnSetup)
            {
                _spawnSetup = true;
                if (_self != null)
                {
                    _self.setAnimationLooping(false);
                    uint frames = _self.getAnimationFrameCount();
                    if (frames > 0) _self.setAnimationFrameDuration(spawnDuration / frames);
                }
            }
            _timer += Time.deltaTime;
            if (_timer >= spawnDuration)
                _self?.changeState("Idle");
        }
        public void Spawn_exit() { }

        // ── Idle ──────────────────────────────────────────────────────────────────
        public void Idle_enter()
        {
            _pathTimer = pathRefreshInterval;
            _stuckTimer = 0f;
            if (_self != null) _lastStuckPos = _self.position;
        }
        public void Idle_update()
        {
            if (PlayerController.IsGamePaused) return;
            FindPlayer();
            if (_self == null || _player == null) return;

            if (Dist(_self.position, _player.position) <= TRIGGER_RADIUS)
            {
                _self.velocity = new Vector2(0f, 0f);
                _self.changeState("WindUp");
                return;
            }

            _pathTimer += Time.deltaTime;
            if (_pathTimer >= pathRefreshInterval)
            {
                _pathTimer = 0f;
                _self.computePath(_player.position);
            }

            // Stuck detection: if barely moved since last check, avoid current waypoint
            // Skip when close to player — let the trigger radius check handle it
            float distToPlayer = Dist(_self.position, _player.position);
            _stuckTimer += Time.deltaTime;
            if (_stuckTimer >= 0.5f)
            {
                float sdx = _self.position.x - _lastStuckPos.x;
                float sdy = _self.position.y - _lastStuckPos.y;
                if (_self.hasPath && sdx * sdx + sdy * sdy < 10f * 10f && distToPlayer > TRIGGER_RADIUS)
                    _self.computePathAvoidCurrentWaypoint(_player.position);
                _lastStuckPos = _self.position;
                _stuckTimer = 0f;
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
                Vector2 dir = Norm(dx, dy);
                _self.velocity = new Vector2(dir.x * chaseSpeed, dir.y * chaseSpeed);
                if (dir.x != 0f)
                {
                    Vector2 s = _self.scale;
                    _self.scale = new Vector2(dir.x > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
                }
            }
        }
        public void Idle_exit() { _self?.clearPath(); }

        // ── WindUp ────────────────────────────────────────────────────────────────
        public void WindUp_enter()
        {
            _timer = 0f;

            if (!_gameEnded && _audio != null && _self != null)
            {
                _audio.sound = WINDUP_SOUND;
                _audio.spatialPlay(_audio.ID, _self.position.x, _self.position.y);
            }
        }
        public void WindUp_update()
        {
            if (PlayerController.IsGamePaused) return;
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
            _dashReady     = false;

            if (!_gameEnded && _audio != null && _self != null)
            {
                _audio.sound = ATTACK_SOUND;
                _audio.spatialPlay(_audio.ID, _self.position.x, _self.position.y);
            }

            if (_self != null)
                _self.velocity = new Vector2(_dashDir.x * dashSpeed, _dashDir.y * dashSpeed);
        }
        public void Attack_update()
        {
            if (PlayerController.IsGamePaused) return;
            if (_self == null) return;

            Vector2 curPos = _self.position;

            // Skip first frame — physics hasn't moved the body yet, so moved≈0
            // would falsely trigger the wall-hit check.
            if (!_dashReady)
            {
                _prevPos   = curPos;
                _dashReady = true;
                return;
            }

            float moved    = Dist(curPos, _prevPos);
            _chargedPixels += moved;
            _prevPos        = curPos;

            float expectedMove = dashSpeed * Time.deltaTime;
            bool  hitWall      = moved < expectedMove * 0.5f;

            if (_chargedPixels >= CHARGE_DISTANCE || hitWall)
            {
                _self.velocity = new Vector2(0f, 0f);
                _self.changeState("Idle");
            }
        }
        public void Attack_exit() { if (_self != null) _self.velocity = new Vector2(0f, 0f); }

        // ── Frozen ────────────────────────────────────────────────────────────────
        public void Frozen_enter()
        {
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        // ── Hurt ──────────────────────────────────────────────────────────────────
        public void Hurt_enter()
        {
            _timer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);

            if (!_gameEnded && _audio != null && _self != null)
            {
                _audio.sound = HURT_SOUND;
                _audio.spatialPlay(_audio.ID, _self.position.x, _self.position.y);
            }
        }
        public void Hurt_update()
        {
            if (PlayerController.IsGamePaused) return;
            _timer += Time.deltaTime;
            if (_timer >= hurtDuration)
            {
                _self?.changeState("Idle");
            }
        }
        public void Hurt_exit() { }

        // ── Death ─────────────────────────────────────────────────────────────────
        public void Death_enter()
        {
            _timer = 0f;
            if (_self == null) return;
            _self.stateChangeLocked = true;
            _self.velocity = new Vector2(0f, 0f);
            if (_hpBarFull != null) _hpBarFull.scale = new Vector2(0f, _hpBarFull.scale.y);
            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            if (frames > 0) _self.setAnimationFrameDuration(deathDuration / frames);

            if (!_gameEnded && _audio != null)
            {
                _audio.sound = DEATH_SOUND;
                _audio.spatialPlay(_audio.ID, _self.position.x, _self.position.y);
            }
        }
        public void Death_update()
        {
            if (_self == null) return;
            _self.velocity = new Vector2(0f, 0f);
            _self.setAnimationLooping(false);
            _timer += Time.deltaTime;
            if (_timer >= deathDuration)
            {
                if (_rng.NextDouble() < dropChance)
                    SpawnPrefab(HEALTH_POTION_PREFAB, _self!.position.x, _self!.position.y);
                _allInstances.Remove(this);
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
