using System;
using System.Collections.Generic;

namespace Eno
{
    public class NullflyBase_Script : ScriptBase
    {
        private static List<NullflyBase_Script> _allInstances = new List<NullflyBase_Script>();
        private static bool _gameEnded = false;

        public static void StopAllAudio()
        {
            _gameEnded = true;
            foreach (var inst in _allInstances)
                inst._audio?.stop();
        }

        public int   runeIndex       = 5;      // 0-5: which rune this Nullfly bans
        public float cloakRadius     = 220f;   // player closer than this → cloak
        public float uncloakRadius   = 300f;   // player farther than this → uncloak
        public float idleDuration    = 0.0f;   // time in Idle before channelling
        public float shockedDuration = 0.2f;
        public float windUpDuration  = 0.3f;
        public float hurtDuration    = 0.5f;
        public float deathDuration   = 1.0f;
        public float prespawnDuration = 1.0f;
        public float spawnDuration   = 0.5f;
        public float contactDamageCooldown = 1.0f;
        public float dropChance = 0.3f;    // Probability [0-1] of dropping a HealthPotion on death

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

        // Set this before SpawnPrefab to assign runeIndex on the next spawned Nullfly
        public static int PendingRuneIndex = 5;

        private const string ATTACK_SOUND        = "Nullfly_Attack_01";
        private const string AUDIO               = "DefaultAudio";
        private const string HEALTH_POTION_PREFAB = "HealthPotion";

        private static readonly Random _rng = new Random();
        private float  _timer          = 0f;
        private bool   _prespawnSetup  = false;
        private bool   _spawnSetup     = false;
        private bool   _isSpawning     = true;
        private float  _contactTimer  = 0f;
        private bool   _isCloaked     = false;
        private bool   _runeBanned    = false;

        // ── Cloak toggle ─────────────────────────────────────────────────────────
        private void SetCloaked(bool cloaked)
        {
            if (_isCloaked == cloaked) return;
            _isCloaked = cloaked;
            if (_self != null) _self.visible = !cloaked;
        }

        // ── Rune ban helpers — flag prevents count from accumulating ─────────────
        private void BanMyRune()
        {
            if (_runeBanned) return;
            _runeBanned = true;
            EnemyColorRegistry.BanRune(runeIndex);
            RuneSystem.NotifyBan(runeIndex);
        }

        private void UnbanMyRune()
        {
            if (!_runeBanned) return;
            _runeBanned = false;
            EnemyColorRegistry.UnbanRune(runeIndex);
            RuneSystem.NotifyUnban(runeIndex);
        }

        // ── Lifecycle ────────────────────────────────────────────────────────────
        public void Begin()
        {
            _self = Self<Enemy>();
            runeIndex = PendingRuneIndex;
            if (_self == null) return;
            
            // Initialize Nullfly Values
            _self.useGravity    = false;
            _self.isKinematic   = true;
            _self.velocity      = new Vector2(0f, 0f);
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

            // Audio node instantiation
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
            if (!_isSpawning && _self.isColliding(_player))
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
            _isSpawning = true;
            if (_self != null)
            {
                _self.velocity = new Vector2(0f, 0f);
                _self.z_index = 1;
            }
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

        // ── Spawn ────────────────────────────────────────────────────────────────
        public void Spawn_enter()
        {
            _timer = 0f;
            _spawnSetup = false;
            if (_self != null) _self.z_index = 3;
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
        public void Spawn_exit() { _isSpawning = false; }

        // ── Idle (visible, rune banned, timed → Channel) ─────────────────────────
        public void Idle_enter()
        {
            _timer = 0f;
            SetCloaked(false);
            UnbanMyRune();   // safety net: ensures ban count is clean on any path into Idle
        }
        public void Idle_update()
        {
            if (PlayerController.IsGamePaused) return;
            _timer += Time.deltaTime;
            if (_timer >= idleDuration)
                _self?.changeState("Channel");
        }
        public void Idle_exit() { }

        // ── Shocked (brief reaction before cloaking) ─────────────────────────────
        public void Shocked_enter() { _timer = 0f; }
        public void Shocked_update()
        {
            if (PlayerController.IsGamePaused) return;
            _timer += Time.deltaTime;
            if (_timer >= shockedDuration)
                _self?.changeState("WindUp");
        }
        public void Shocked_exit() { }

        // ── WindUp (cloak + unban, then monitor distance → Idle) ────────────────
        public void WindUp_enter()
        {
            _timer = 0f;

            if (!_gameEnded && _audio != null && _self != null)
            {
                _audio.sound = ATTACK_SOUND;
                _audio.spatialPlay(_audio.ID, _self.position.x, _self.position.y);
            }
        }
        public void WindUp_update()
        {
            if (PlayerController.IsGamePaused) return;
            if (!_isCloaked)
            {
                _timer += Time.deltaTime;
                if (_timer >= windUpDuration)
                {
                    SetCloaked(true);
                    UnbanMyRune();
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
                UnbanMyRune();
        }

        // ── Channel (visible, always bans rune; player near → Shocked) ───────────
        public void Channel_enter()
        {
            BanMyRune();
        }
        public void Channel_update()
        {
            if (PlayerController.IsGamePaused) return;
            FindPlayer();
            if (_self == null || _player == null) return;
            if (Dist(_self.position, _player.position) <= cloakRadius)
                _self.changeState("Shocked");
        }
        public void Channel_exit() { }

        // ── Frozen ───────────────────────────────────────────────────────────────
        public void Frozen_enter()
        {
            if (_self != null) _self.velocity = new Vector2(0f, 0f);
        }
        public void Frozen_update() { }
        public void Frozen_exit() { }

        // ── Hurt ─────────────────────────────────────────────────────────────────
        public void Hurt_enter() { _timer = 0f; }
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

        // ── Death ────────────────────────────────────────────────────────────────
        public void Death_enter()
        {
            _timer = 0f;
            if (_self == null) return;
            _self.stateChangeLocked = true;
            if (_hpBarFull != null) _hpBarFull.scale = new Vector2(0f, _hpBarFull.scale.y);
            _self.setAnimationLooping(false);
            uint frames = _self.getAnimationFrameCount();
            if (frames > 0) _self.setAnimationFrameDuration(deathDuration / frames);
            if (_isCloaked)
            {
                _isCloaked = false;
                if (_self != null) _self.visible = true;
            }
            UnbanMyRune(); // safe no-op if already unbanned
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
