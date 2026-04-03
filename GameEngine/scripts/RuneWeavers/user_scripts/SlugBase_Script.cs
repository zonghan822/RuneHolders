using System;
using System.Collections.Generic;

namespace Eno
{
    public class SlugBase_Script : ScriptBase
    {
        private static List<SlugBase_Script> _allInstances = new List<SlugBase_Script>();
        private static bool _gameEnded = false;

        // Called when win/defeat screen appears — silences all walk loops and prevents restart
        public static void StopAllWalkAudio()
        {
            _gameEnded = true;
            foreach (var inst in _allInstances)
                inst._walkAudio?.stop();
        }

        public float chaseSpeed          = 60f;
        public float pathRefreshInterval = 0.5f;
        public float hurtDuration        = 0.4f;
        public float deathDuration       = 1.0f;
        public float prespawnDuration    = 1.0f;
        public float spawnDuration       = 0.5f;
        public float contactDamageCooldown = 1.0f;
        public float dropChance = 0.3f;    // Probability [0-1] of dropping a HealthPotion on death

        protected virtual int EnemyColor => 0;

        private Enemy?    _self;
        private Actor?    _player;
        private AudioNode? _walkAudio;      // Constant walk sound
        private AudioNode? _hdAudio;        // Hurt | death audio 
        private Sprite2D? _hpBarEmpty;
        private Sprite2D? _hpBarFull;
        private float     _hpBarFullInitialScaleX = 1f;
        private float     _hpBarFullHalfWidth     = 0f;
        private bool      _playerSearched = false;

        private const string HP_BAR_EMPTY_PREFAB = "EnemyHealthBar_Empty";
        private const string HP_BAR_FULL_PREFAB  = "EnemyHealthBar_Full";
        private const float  HP_BAR_OFFSET_Y     = -50f;

        private const string AUDIO = "DefaultAudio";

        List<string> _deathSounds = new List<string>
        {
          "Trailblight_Death_01",
          "Trailblight_Death_02",
          "Trailblight_Death_03"
        };

        List<string> _hurtSounds = new List<string>
        {
          "Trailblight_Hurt_01",
          "Trailblight_Hurt_02",
          "Trailblight_Hurt_03",
          "Trailblight_Hurt_04",
          "Trailblight_Hurt_05",
          "Trailblight_Hurt_06"
        };

        private const string MOVE_SOUND          = "Trailblight_Move_Loop";
        private const string HEALTH_POTION_PREFAB = "HealthPotion";
        
        private float  _pathTimer       = 0f;
        private bool   _prespawnSetup   = false;
        private bool   _spawnSetup      = false;
        private float  _hurtTimer       = 0f;
        private float  _deathTimer      = 0f;
        private float  _spawnTimer      = 0f;
        private float  _contactTimer    = 0f;
        private float  _stuckTimer      = 0f;
        private Vector2 _lastStuckPos   = new Vector2(0f, 0f);

        private Random rng = new Random();

        public void Begin()
        {
            _self = Self<Enemy>();
            if (_self == null) return;
            _allInstances.Add(this);
            
            // Initialize Slug Values
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

            // Audio
            _walkAudio = SpawnPrefab(AUDIO, _self.position.x, _self.position.y, _self) as AudioNode;
            _hdAudio = SpawnPrefab(AUDIO, _self.position.x, _self.position.y, _self) as AudioNode; 
            
            _walkAudio.volume = 30f;  // capped at 30 — sliders/spatial can only attenuate further
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

        public void Prespawn_enter()
        {
            _spawnTimer = 0f;
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
            _spawnTimer += Time.deltaTime;
            if (_spawnTimer >= prespawnDuration)
                _self?.changeState("Spawn");
        }
        public void Prespawn_exit() { }

        public void Spawn_enter()
        {
            _spawnTimer = 0f;
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
            if (PlayerController.IsGamePaused) return;
            _self?.changeState("Chase");
        }
        public void Idle_exit() { }

        public void Chase_enter()
        {
            _pathTimer = pathRefreshInterval;
            _stuckTimer = 0f;
            if (_self != null) _lastStuckPos = _self.position;

            if (_walkAudio != null && _self != null)
            {
                _walkAudio.sound = MOVE_SOUND;
                _walkAudio.spatialPlay(_walkAudio.ID, _self.position.x, _self.position.y);
            }
        }

        public void Chase_update()
        {
            if (PlayerController.IsGamePaused || _gameEnded) return;
            FindPlayer();
            if (_self == null || _player == null) return;

            _pathTimer += Time.deltaTime;
            if (_pathTimer >= pathRefreshInterval)
            {
                _pathTimer = 0f;
                _self.computePath(_player.position);
            }

            // Stuck detection: if barely moved since last check, avoid current waypoint
            _stuckTimer += Time.deltaTime;
            if (_stuckTimer >= 0.5f)
            {
                float sdx = _self.position.x - _lastStuckPos.x;
                float sdy = _self.position.y - _lastStuckPos.y;
                if (_self.hasPath && sdx * sdx + sdy * sdy < 10f * 10f)
                    _self.computePathAvoidCurrentWaypoint(_player.position);
                _lastStuckPos = _self.position;
                _stuckTimer = 0f;
            }

            // Loop walk audio: replay spatially whenever the previous play finishes
            if (_walkAudio != null && !_walkAudio.isPlaying)
            {
                _walkAudio.sound = MOVE_SOUND;
                _walkAudio.spatialPlay(_walkAudio.ID, _self.position.x, _self.position.y);
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
            _walkAudio?.stop();
        }

        public void Frozen_enter()
        {
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        public void Hurt_enter()
        {
            Bridge.AudioPlay(ID);
            _hurtTimer = 0f;
            if (_self != null) _self.velocity = new Vector2(0f, 0f);

            if (_hdAudio != null && _self != null)
            {
                int i = rng.Next(0, _hurtSounds.Count);
                Console.WriteLine($"Sound selected: { _hurtSounds[i] }");
                _hdAudio.sound = _hurtSounds[i];
                _hdAudio.spatialPlay(_hdAudio.ID, _self.position.x, _self.position.y);
            }
        }
        public void Hurt_update()
        {
            if (PlayerController.IsGamePaused) return;
            _hurtTimer += Time.deltaTime;
            if (_hurtTimer >= hurtDuration)
            {
                _self?.changeState("Idle");
            }
        }
        public void Hurt_exit() { }

        public void Death_enter()
        {
            _deathTimer = 0f;
            if (_self == null) return;
            _self.stateChangeLocked = true;
            _self.velocity = new Vector2(0f, 0f);
            if (_hpBarFull != null) _hpBarFull.scale = new Vector2(0f, _hpBarFull.scale.y);
            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            if (frames > 0) _self.setAnimationFrameDuration(deathDuration / frames);

            if (_hdAudio != null)
            {
                int i = rng.Next(0, _deathSounds.Count);
                _hdAudio.sound = _deathSounds[i];
                _hdAudio.spatialPlay(_hdAudio.ID, _self.position.x, _self.position.y);
            }
        }
        public void Death_update()
        {
            if (_self == null) return;
            _self.velocity = new Vector2(0f, 0f);
            _self.setAnimationLooping(false);
            _deathTimer += Time.deltaTime;

            if (_deathTimer >= deathDuration)
            {
                if (rng.NextDouble() < dropChance)
                    SpawnPrefab(HEALTH_POTION_PREFAB, _self!.position.x, _self!.position.y);
                _allInstances.Remove(this);
                EnemyColorRegistry.Unregister(ID);
                FreeNode(_self!);
            }
        }
        public void Death_exit() { }

        private void FindPlayer()
        {
            if (_playerSearched) return;
            _playerSearched = true;
            _player = this.GetNode<Actor>("Player");
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
