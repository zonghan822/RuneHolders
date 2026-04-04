/*============================================================================================
* @file         RuneSystem.cs
* @brief        Handles the rune circle spell-casting mechanic.
*
* Cast flow:
*   1. Mouse held  → SpellBase (ring visual) spawns at origin; rune nodes appear.
*   2. First rune connected → the matching SpellCasting prefab spawns at origin;
*      its UV advances one column per subsequent connection (charge visualisation).
*   3. Final rune connected → SpellBase + SpellCasting despawn; the spell prefab
*      (RedSpell / YellowSpell / BlueSpell) spawns and runs SpellEffect.cs.
*   4. Mouse released early or right-click → cancel, all runes Nullified, cleaned up.
*
* Rune layout (clockwise from upper-right):
*
*          5 0
*        4     1
*          3 2
*
* Spell patterns:
*   FIRE   – 0 → 1 → 2 → 3
*   LIGHT  – 5 → 4 → 1 → 2
*   ICE    – 4 → 3 → 5 → 0
*
* Rune states:
*   Disabled  – not a valid next step in any remaining pattern
*   Ready     – a valid next connection in at least one remaining pattern
*   Activated – already connected by the player this cast
*   Nullified – blocked by an enemy; patterns requiring this rune are eliminated
*
* Required prefabs:
*   "Runes"              – 6-column spritesheet; UV column i selects the rune glyph.
*                         States: Disabled | Ready | Activated | Nullified
*   "SpellBase"         – ring / aura shown at origin while casting
*   "FireSpawn"         – charge indicator for RED; spritesheet has SPELL_STEPS columns
*   "LightSpawn"        – charge indicator for YELLOW
*   "IceSpawn"          – charge indicator for BLUE
*   "FireSpell"         – final spell; attach SpellEffect.cs; states: windup | explosion
*   "LightSpell"        – final spell
*   "IceSpell"          – final spell
*============================================================================================*/

#nullable enable
using System;
using System.Collections.Generic;

namespace Eno
{
    public class RuneSystem : ScriptBase
    {
        // ── Tuneable values ──────────────────────────────────────────────────────
        private const float  RUNE_RING_RADIUS = 100f;
        private const float  SNAP_RADIUS      = 28f;
        private const int    RUNE_COUNT       = 6;
        private const string LINE_PREFAB      = "RuneLine";
        private const float  LINE_HEIGHT      = 16f;

        // ── StopWatch / slow-time settings ───────────────────────────────────
        public float  slowTimeScale    = 0.3f;
        public string stopWatchNodeName = "StopWatch";

        // ── Player settings ──────────────────────────────────────────────────
        public string playerNodeName = "Player";

        // ── Spell definitions ────────────────────────────────────────────────────
        private static readonly int[][] Patterns = new int[][]
        {
            new int[] { 0, 1, 2, 3 },    // FIRE    – 0 → 1 → 2 → 3
            new int[] { 5, 4, 1, 2 },    // LIGHT   – 5 → 4 → 1 → 2
            new int[] { 4, 3, 5, 0 },    // ICE     – 4 → 3 → 5 → 0
        };

        // Spritesheet columns per SpellCasting prefab = Patterns[s].Length
        private static readonly string[] SpellCastingPrefabs = new string[]
        {
            "FireSpawn",
            "LightSpawn",
            "IceSpawn",
        };

        private static readonly string[] SpellPrefabs = new string[]
        {
            "FireSpell",
            "LightSpell",
            "IceSpell",
        };

        // ── Spell unlock state (set by room scripts on scroll pickup) ────────────
        // Index 0 = Fire (Room 1), 1 = Light (Room 4), 2 = Ice (Room 8)
        public static bool[] SpellUnlocked = new bool[3];

        // Set to true by CastSpell() each time the player successfully fires a spell.
        // Consuming scripts should read this flag and reset it to false.
        public static bool SpellCastOccurred = false;

        // ── Static instance — allows enemies to push bans mid-cast ───────────────
        private static RuneSystem? _instance;

        // Called by Nullfly Channel_enter to nullify a rune even while casting is active.
        public static void NotifyBan(int runeIndex)
        {
            _instance?.EnemyNullifyRune(runeIndex);
        }

        // Called by Nullfly when it cloaks (unbans) while casting is active.
        public static void NotifyUnban(int runeIndex)
        {
            _instance?.EnemyRestoreRune(runeIndex);
        }

            // ── Controller settings ──────────────────────────────────────────────────
        private const float RT_THRESHOLD     = 0.5f;   // trigger value (0-1) to start casting
        private const float STICK_ACTIVATE   = 0.7f;   // right stick magnitude to activate a rune

        // Precomputed rune angles (radians) for stick-based activation
        private static readonly float[] RuneAngles = new float[RUNE_COUNT];
        private static readonly bool _anglesInit = InitRuneAngles();
        private static bool InitRuneAngles()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
                RuneAngles[i] = (-60f + i * (360f / RUNE_COUNT)) * MathF.PI / 180f;
            return true;
        }

        // ── Runtime state ────────────────────────────────────────────────────────
        private bool        _active         = false;
        private bool        _rtWasDown      = false;    // previous frame RT state
        private float       _originX        = 0f;
        private float       _originY        = 0f;

        // Rune ring
        private float[]     _runeX          = new float[RUNE_COUNT];
        private float[]     _runeY          = new float[RUNE_COUNT];
        private Sprite2D?[] _runes          = new Sprite2D?[RUNE_COUNT];
        private string[]    _runeStates     = new string[RUNE_COUNT];
        private List<int>   _connected      = new();

        // Casting visuals
        private KinematicBody2D? _spellBase  = null;   // ring at origin
        private Sprite2D?   _spellCasting   = null;   // charge indicator
        private int         _activePattern  = -1;     // index into Patterns[], -1 = undetermined

        // Connection lines
        private List<Sprite2D?> _lines     = new();   // one permanent line per confirmed connection
        private Sprite2D?       _trailLine = null;    // live line from last connection to mouse

        // StopWatch / slow-time — cached on first use (null = not found in scene)
        private Sprite2D? _swSprite   = null;
        private bool      _swSearched = false;

        // Player — cached on first use
        private Actor? _player        = null;
        private bool   _playerSearched = false;

        // Particle blink effect — one per rune, spawned as children
        private ParticleEmitter?[] _runeParticles = new ParticleEmitter?[RUNE_COUNT];

        // Deferred spell cast — pattern index stored when complete, cast on mouse release
        private int _pendingSpell = -1;

        // Short rumble pulse on rune connection
        private float _rumbleTimer    = 0f;
        private const float RUMBLE_DURATION = 0.1f;

        // Set when an enemy force-cancels the cast; cleared on full input release
        private bool _castBlocked = false;

        // Cursor prefab — spawned once, tracks mouse / right stick
        private Sprite2D? _cursor = null;

        // ── Audio ────────────────────────────────────────────────────────────────
        public float runeActiveVolume = 70f;   // volume for the Rune-Active loop (0-100)
        public float spellCastVolume  = 80f;   // volume for the SpellCast one-shot (0-100)

        // Rune activation audio — one dedicated node per rune (index matches connection order)
        private AudioNode?[] _runeAudios = new AudioNode?[RUNE_COUNT];
        private List<string> _bellPadSounds = new List<string>
        {
            "Rune-GalaxyBellPad-1",
            "Rune-GalaxyBellPad-2",
            "Rune-GalaxyBellPad-3",
            "Rune-GalaxyBellPad-4",
            "Rune-GalaxyBellPad-5",
            "Rune-GalaxyBellPad-6",
        };

        // Rune-Active loop — plays while the rune circle is open
        private AudioNode? _runeActiveAudio = null;

        // SpellCast one-shot — plays on successful cast
        private AudioNode? _spellCastAudio = null;
        private List<string> _spellCastSounds = new List<string>
        {
            "SpellCast_01",
            "SpellCast_02",
            "SpellCast_03",
        };

        private Random rng = new Random();

        // ── Script callbacks ─────────────────────────────────────────────────────
        public void Begin()
        {
            _instance = this;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                _runes[i]      = null;
                _runeStates[i] = "Disabled";
            }

            for (int s = 0; s < 3; s++)
            {
                SpellUnlocked[s] = false;
            }

            _cursor = SpawnPrefab("Cursor", 0f, 0f) as Sprite2D;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                _runeAudios[i] = SpawnPrefab("DefaultBGM", 0f, 0f) as AudioNode;
                if (_runeAudios[i] != null) _runeAudios[i]!.volume = 80f;
            }

            _runeActiveAudio = SpawnPrefab("DefaultBGM", 0f, 0f) as AudioNode;
            if (_runeActiveAudio != null) _runeActiveAudio.volume = runeActiveVolume;

            _spellCastAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            if (_spellCastAudio != null) _spellCastAudio.volume = spellCastVolume;
        }

        public void Update()
        {
            if (PlayerController.IsGameCompleted) return;

            // Drain short rumble pulse
            if (_rumbleTimer > 0f)
            {
                _rumbleTimer -= Time.deltaTime;
                if (_rumbleTimer <= 0f)
                    Input.setGamepadRumble(0f, 0f);
            }

            if (PlayerController.IsGamePaused || Scroll_Dialouge_Script.DialogueActive)
            {
                if (_active) Cancel();
                _rtWasDown = false;
                return;
            }

            // Controller: RT as activation trigger (0-1 range)
            bool rtDown = Input.getRightTrigger() > RT_THRESHOLD;
            bool rtPressed = rtDown && !_rtWasDown;
            bool rtReleased = !rtDown && _rtWasDown;
            _rtWasDown = rtDown;

            if (!_active)
            {
                if (_castBlocked)
                {
                    // Wait for the player to fully release before allowing a new cast
                    if (!Input.isMouseHeld(0) && !rtDown)
                        _castBlocked = false;
                }
                else if ((Input.isMousePressed(0) && !Scroll_Dialouge_Script.DialogueActive) || rtPressed)
                {
                    Activate();
                }
            }
            else
            {
                if (Input.isMouseHeld(0) || rtDown)
                {
                    if (_pendingSpell < 0)
                    {
                        // Right stick → angle-based activation; mouse → position-based
                        Vector2 rStick = Input.getRightStick(0.15f);
                        float stickMag = MathF.Sqrt(rStick.x * rStick.x + rStick.y * rStick.y);

                        if (stickMag >= STICK_ACTIVATE)
                            CheckConnectionByAngle(rStick.x, rStick.y);
                        else
                            CheckConnectionsAt(Cursor_Script.CursorX, Cursor_Script.CursorY);

                        // Trail line follows stick direction, or snaps to origin when released
                        if (stickMag > 0.15f)
                        {
                            float norm = RUNE_RING_RADIUS / stickMag;
                            UpdateTrailLine(_originX + rStick.x * norm, _originY + rStick.y * norm);
                        }
                        else if (rtDown)
                        {
                            UpdateTrailLine(_originX, _originY);
                        }
                        else
                        {
                            UpdateTrailLine(Cursor_Script.CursorX, Cursor_Script.CursorY);
                        }
                    }
                    else
                    {
                        // Pattern complete — hide trail line
                        if (_trailLine != null) { FreeNode(_trailLine); _trailLine = null; }
                    }
                }
                else if (rtReleased)
                {
                    Cursor_Script.CursorX = _originX;
                    Cursor_Script.CursorY = _originY;
                    if (_pendingSpell >= 0)
                        Release();
                    else
                        Cancel();
                }
                else
                    Release();

                if (Input.isMousePressed(1) || Input.isGamepadButtonPressed(GamepadButton.B))
                    Cancel();
            }

            // Keep Rune-Active looping only after all 4 runes are connected and player is holding
            if (_active && _pendingSpell >= 0 && _runeActiveAudio != null && !_runeActiveAudio.isPlaying)
            {
                _runeActiveAudio.sound = "Rune-Active";
                _runeActiveAudio.play();
            }
        }

        // ── Enemy API ────────────────────────────────────────────────────────────

        // Called by an enemy to corrupt a rune by its layout index (0–5).
        public void EnemyNullifyRune(int runeIndex)
        {
            if (!_active) return;
            if (runeIndex < 0 || runeIndex >= RUNE_COUNT) return;
            if (_runes[runeIndex] == null) return;
            if (_runeStates[runeIndex] == "Nullified") return;

            // If the player already activated this rune, break the entire cast
            if (_runeStates[runeIndex] == "Activated")
            {
                _castBlocked = true;
                Cancel();
                return;
            }

            SetRuneState(runeIndex, "Nullified");
            RefreshReadyStates();
        }

        // Called when a Nullfly cloaks mid-cast — restores the rune from Nullified.
        private void EnemyRestoreRune(int runeIndex)
        {
            if (!_active) return;
            if (runeIndex < 0 || runeIndex >= RUNE_COUNT) return;
            if (_runes[runeIndex] == null) return;
            if (_runeStates[runeIndex] != "Nullified") return;

            HashSet<int> nextValid = GetNextValidRunes();
            SetRuneState(runeIndex, nextValid.Contains(runeIndex) ? "Ready" : "Disabled");
        }

        // ── Private helpers ──────────────────────────────────────────────────────

        private void Activate()
        {
            _originX       = Cursor_Script.CursorX;
            _originY       = Cursor_Script.CursorY;
            _activePattern = -1;
            _pendingSpell  = -1;
            _connected.Clear();

            if (_cursor != null) _cursor.visible = false;

            _spellBase = SpawnPrefab("SpellBase", _originX, _originY) as KinematicBody2D;

            SpawnRunes();
            _active = true;

            // Apply rune bans from any visible Nullfly enemies
            foreach (int runeIdx in EnemyColorRegistry.ActiveBans)
                EnemyNullifyRune(runeIdx);

            // Slow world time and freeze enemies overlapping the spell base
            Time.timeScale = slowTimeScale;
            Bridge.SetGroupLowPass("SFX", 600f);
            if (_spellBase != null)
            {
                foreach (Enemy enemy in GetAllEnemies())
                {
                    if (_spellBase.isColliding(enemy) && enemy.currentState != "Prespawn")
                        enemy.changeState("Frozen");
                }
            }

            // Show the StopWatch sprite and trigger fade-in
            FindStopWatch();
            if (_swSprite != null)
                _swSprite.visible = true;
            _swSprite?.changeState("FadeIn");

            // Set player state to Cast or CastWalk depending on movement
            FindPlayer();
            _player?.changeState(IsPlayerMoving() ? "CastWalk" : "Cast");
        }

        private void SpawnRunes()
        {
            HashSet<int> nextValid = GetNextValidRunes();

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                float angleDeg = -60f + i * (360f / RUNE_COUNT);
                float angleRad = angleDeg * MathF.PI / 180f;

                _runeX[i] = _originX + RUNE_RING_RADIUS * MathF.Cos(angleRad);
                _runeY[i] = _originY + RUNE_RING_RADIUS * MathF.Sin(angleRad);

                _runes[i] = SpawnPrefab("Runes", _runeX[i], _runeY[i]) as Sprite2D;
                if (_runes[i] == null) continue;

                float uvW = 1.0f / RUNE_COUNT;
                _runes[i]!.setUV(new Vector2(i * uvW, 0.0f), new Vector2((i + 1) * uvW, 1.0f));

                string state = nextValid.Contains(i) ? "Ready" : "Disabled";
                _runes[i]!.changeState(state);
                _runeStates[i] = state;

                // Spawn particle emitter as child of the rune
                _runeParticles[i] = SpawnPrefab("RuneParticles", _runeX[i], _runeY[i]) as ParticleEmitter;
                if (_runeParticles[i] != null)
                {
                    _runes[i]!.AddChild(_runeParticles[i]);
                    _runeParticles[i]!.stopEmission();
                }
            }
        }

        private void CheckConnectionsAt(float mx, float my)
        {
            float snapSq = SNAP_RADIUS * SNAP_RADIUS;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runeStates[i] != "Ready") continue;

                float dx = mx - _runeX[i];
                float dy = my - _runeY[i];
                if (dx * dx + dy * dy > snapSq) continue;

                _connected.Add(i);
                SetRuneState(i, "Activated");
                PlayBellPad(i, _connected.Count);

                // Short rumble pulse — intensity increases with each connection
                float rumble = MathF.Min(0.2f + _connected.Count * 0.15f, 0.8f);
                Input.setGamepadRumble(rumble, rumble);
                _rumbleTimer = RUMBLE_DURATION;

                // Spawn a permanent line from the previous rune to this one (skip on first connection)
                if (_connected.Count > 1)
                {
                    int prev = _connected[_connected.Count - 2];
                    SpawnConnectionLine(_runeX[prev], _runeY[prev], _runeX[i], _runeY[i]);
                }

                // On first connection: despawn SpellBase and spawn the matching casting prefab
                if (_connected.Count == 1)
                {
                    if (_spellBase != null) { FreeNode(_spellBase); _spellBase = null; }
                    TryDeterminePattern();
                }

                // Advance the charge visualisation by changing state
                _spellCasting?.changeState("Cast" + _connected.Count);

                // If the full pattern is now complete, store it and emit blink on all runes
                if (_activePattern >= 0 && PatternMatches(_connected, Patterns[_activePattern]))
                {
                    _pendingSpell = _activePattern;

                    if (_runeActiveAudio != null)
                    {
                        _runeActiveAudio.volume = runeActiveVolume;
                        _runeActiveAudio.sound = "Rune-Active";
                        _runeActiveAudio.play();
                    }

                    Input.setGamepadRumble(1.0f, 1.0f);
                    _rumbleTimer = RUMBLE_DURATION;
                    foreach (int c in _connected)
                        _runeParticles[c]?.emitBurst(_runeParticles[c].maxParticles/4);
                    return;
                }

                RefreshReadyStates();
            }
        }

        // Activate the Ready rune closest to the right stick direction.
        private void CheckConnectionByAngle(float sx, float sy)
        {
            float stickAngle = MathF.Atan2(sy, sx);

            // Find the Ready rune with the smallest angular distance
            int best = -1;
            float bestDist = float.MaxValue;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runeStates[i] != "Ready") continue;

                float diff = stickAngle - RuneAngles[i];
                // Wrap to [-PI, PI]
                diff = MathF.Atan2(MathF.Sin(diff), MathF.Cos(diff));
                float absDiff = MathF.Abs(diff);

                // Only match if within 30° (half the 60° sector between runes)
                if (absDiff < bestDist && absDiff < MathF.PI / 6f)
                {
                    bestDist = absDiff;
                    best = i;
                }
            }

            if (best < 0) return;

            // Activate the matched rune (same logic as CheckConnectionsAt)
            _connected.Add(best);
            SetRuneState(best, "Activated");
            PlayBellPad(best, _connected.Count);

            // Short rumble pulse — intensity increases with each connection
            float rumble = MathF.Min(_connected.Count * 0.1f, 0.5f);
            Input.setGamepadRumble(rumble, rumble);
            _rumbleTimer = RUMBLE_DURATION;

            if (_connected.Count > 1)
            {
                int prev = _connected[_connected.Count - 2];
                SpawnConnectionLine(_runeX[prev], _runeY[prev], _runeX[best], _runeY[best]);
            }

            if (_connected.Count == 1)
            {
                if (_spellBase != null) { FreeNode(_spellBase); _spellBase = null; }
                TryDeterminePattern();
            }

            _spellCasting?.changeState("Cast" + _connected.Count);

            if (_activePattern >= 0 && PatternMatches(_connected, Patterns[_activePattern]))
            {
                _pendingSpell = _activePattern;

                if (_runeActiveAudio != null)
                {
                    _runeActiveAudio.volume = runeActiveVolume;
                    _runeActiveAudio.sound = "Rune-Active";
                    _runeActiveAudio.play();
                }

                Input.setGamepadRumble(0.5f, 0.5f);
                _rumbleTimer = RUMBLE_DURATION;
                foreach (int c in _connected)
                    _runeParticles[c]?.emitBurst(_runeParticles[c].maxParticles / 4);
                return;
            }

            RefreshReadyStates();
        }

        // Checks whether only one viable pattern remains and spawns its casting prefab.
        private void TryDeterminePattern()
        {
            int determined = GetViablePatternIndex();
            if (determined == _activePattern) return;

            // Pattern just resolved — swap the casting visual
            if (_spellCasting != null)
            {
                FreeNode(_spellCasting);
                _spellCasting = null;
            }

            _activePattern = determined;

            if (_activePattern >= 0)
                _spellCasting = SpawnPrefab(SpellCastingPrefabs[_activePattern], _originX, _originY) as Sprite2D;
        }

        private void CastSpell(int spellIndex)
        {
            SpellCastOccurred = true;
            _active = false;
            _pendingSpell = -1;
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            SpawnPrefab(SpellPrefabs[spellIndex], _originX, _originY);
            if (_cursor != null) _cursor.visible = true;
            Input.setGamepadRumble(0.8f, 1.0f);
            EndSlowTime(attack: true);

            // Cut Rune-Active immediately and play a random SpellCast
            _runeActiveAudio?.stop();
            if (_spellCastAudio != null)
            {
                int i = rng.Next(0, _spellCastSounds.Count);
                _spellCastAudio.volume = spellCastVolume;
                _spellCastAudio.sound = _spellCastSounds[i];
                _spellCastAudio.play();
            }
        }

        private void Release()
        {
            if (_pendingSpell >= 0)
            {
                int spell = _pendingSpell;
                _pendingSpell = -1;
                CastSpell(spell);
                return;
            }

            _active = false;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            if (_cursor != null) _cursor.visible = true;
            Input.setGamepadRumble(0f, 0f);
            EndSlowTime();
            _runeActiveAudio?.stop();
        }

        private void Cancel()
        {
            _active = false;
            _pendingSpell = -1;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            if (_cursor != null) _cursor.visible = true;
            Input.setGamepadRumble(0f, 0f);
            EndSlowTime();
            _runeActiveAudio?.stop();
        }

        // Restore normal time, unfreeze enemies, and trigger StopWatch fade-out.
        // attack: true when called from CastSpell — sets player to "Attack"; false restores Idle/Walk.
        private void EndSlowTime(bool attack = false)
        {
            Time.timeScale = 1.0f;
            Bridge.SetGroupLowPass("SFX", 0f);
            foreach (Enemy enemy in GetAllEnemies())
            {
                if (enemy.currentState == "Frozen")
                    enemy.changeState("Idle");
            }
            _swSprite?.changeState("FadeOut");

            if (attack)
                _player?.changeState("Attack");
            else
                _player?.changeState(IsPlayerMoving() ? "Walk" : "Idle");
        }

        // Locate the StopWatch sprite by name (cached after first search).
        private void FindStopWatch()
        {
            if (_swSearched) return;
            _swSearched = true;

            try { _swSprite = GetNode<Sprite2D>(stopWatchNodeName); }
            catch { _swSprite = null; }
        }

        // Locate the player Actor by name (cached after first search).
        private void FindPlayer()
        {
            if (_playerSearched) return;
            _playerSearched = true;

            try { _player = GetNode<Actor>(playerNodeName); }
            catch { _player = null; }
        }

        private static bool IsPlayerMoving()
        {
            if (Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.S) ||
                Input.isKeyHeld(KeyCode.A) || Input.isKeyHeld(KeyCode.D))
                return true;

            Vector2 stick = Input.getLeftStick(0.2f);
            return stick.x != 0f || stick.y != 0f;
        }

        // ── Pattern helpers ──────────────────────────────────────────────────────

        // Returns the index of the single remaining viable pattern, or -1 if ambiguous / none.
        private int GetViablePatternIndex()
        {
            int found = -1;
            int depth = _connected.Count;

            for (int s = 0; s < Patterns.Length; s++)
            {
                if (!SpellUnlocked[s]) continue;

                var pattern = Patterns[s];
                if (depth > pattern.Length) continue;

                bool ok = true;
                for (int i = 0; i < depth; i++)
                    if (_connected[i] != pattern[i]) { ok = false; break; }
                if (!ok) continue;

                if (found != -1) return -1;  // more than one viable → undetermined
                found = s;
            }

            return found;
        }

        // Returns rune indices that are valid next connections across all viable patterns.
        private HashSet<int> GetNextValidRunes()
        {
            var result = new HashSet<int>();
            int depth  = _connected.Count;

            // If the pattern is already determined, only check that one
            int lo = _activePattern >= 0 ? _activePattern : 0;
            int hi = _activePattern >= 0 ? _activePattern + 1 : Patterns.Length;

            for (int s = lo; s < hi; s++)
            {
                if (!SpellUnlocked[s]) continue;

                var pattern = Patterns[s];
                if (depth >= pattern.Length) continue;

                bool prefixOk = true;
                for (int i = 0; i < depth; i++)
                    if (_connected[i] != pattern[i]) { prefixOk = false; break; }
                if (!prefixOk) continue;

                int next = pattern[depth];
                if (_runeStates[next] == "Nullified") continue;

                result.Add(next);
            }

            return result;
        }

        // Re-evaluates the Ready / Disabled state of every unconnected, non-nullified rune.
        private void RefreshReadyStates()
        {
            HashSet<int> nextValid = GetNextValidRunes();

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runes[i] == null) continue;
                if (_runeStates[i] == "Activated") continue;
                if (_runeStates[i] == "Nullified") continue;

                string desired = nextValid.Contains(i) ? "Ready" : "Disabled";
                if (_runeStates[i] != desired)
                    SetRuneState(i, desired);
            }
        }

        private void SetRuneState(int index, string state)
        {
            _runes[index]?.changeState(state);
            _runeStates[index] = state;
        }

        private void NullifyUnconnected()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runes[i] == null) continue;
                if (_runeStates[i] == "Activated") continue;
                SetRuneState(i, "Nullified");
            }
        }

        // ── Line helpers ─────────────────────────────────────────────────────────

        private void SpawnConnectionLine(float x1, float y1, float x2, float y2)
        {
            var line = SpawnPrefab(LINE_PREFAB, (x1 + x2) * 0.5f, (y1 + y2) * 0.5f) as Sprite2D;
            if (line == null) return;
            SetLineTransform(line, x1, y1, x2, y2);
            _lines.Add(line);
        }

        private void UpdateTrailLine(float mouseX, float mouseY)
        {
            if (_connected.Count == 0)
                return;

            float startX = _runeX[_connected[_connected.Count - 1]];
            float startY = _runeY[_connected[_connected.Count - 1]];

            if (_trailLine == null)
                _trailLine = SpawnPrefab(LINE_PREFAB, startX, startY) as Sprite2D;

            if (_trailLine != null)
                SetLineTransform(_trailLine, startX, startY, mouseX, mouseY);
        }

        private void SetLineTransform(Sprite2D line, float x1, float y1, float x2, float y2)
        {
            float dx    = x2 - x1;
            float dy    = y2 - y1;
            float dist  = MathF.Sqrt(dx * dx + dy * dy);
            float angle = MathF.Atan2(dy, dx);

            line.position = new Vector2((x1 + x2) * 0.5f, (y1 + y2) * 0.5f);
            line.setSize(dist, LINE_HEIGHT);
            line.rotation = new Vector2(angle, 0f);
        }

        private void DespawnLines()
        {
            foreach (var line in _lines)
                if (line != null) FreeNode(line);
            _lines.Clear();

            if (_trailLine != null) { FreeNode(_trailLine); _trailLine = null; }
        }

        private void DespawnCastingVisuals()
        {
            if (_spellBase != null)
            {
                FreeNode(_spellBase);
                _spellBase = null;
            }
            if (_spellCasting != null)
            {
                FreeNode(_spellCasting);
                _spellCasting  = null;
                _activePattern = -1;
            }
        }

        private void DespawnRunes()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
            {
                // Particle emitters are children of runes — freed with parent
                _runeParticles[i] = null;
                if (_runes[i] != null)
                {
                    FreeNode(_runes[i]);
                    _runes[i]      = null;
                    _runeStates[i] = "Disabled";
                }
            }
            _connected.Clear();
        }

        // runeIndex: layout index of the rune (0-5), selects its dedicated audio node
        // connectionCount: 1-based order of this connection, selects GalaxyBellPad-N sound
        private void PlayBellPad(int runeIndex, int connectionCount)
        {
            if (runeIndex < 0 || runeIndex >= RUNE_COUNT) return;
            var audio = _runeAudios[runeIndex];
            if (audio == null) return;
            int soundIdx = Math.Min(connectionCount, _bellPadSounds.Count) - 1;
            audio.sound = _bellPadSounds[soundIdx];
            audio.play();
        }

        private static bool PatternMatches(List<int> connected, int[] pattern)
        {
            if (connected.Count != pattern.Length) return false;
            for (int i = 0; i < pattern.Length; i++)
                if (connected[i] != pattern[i]) return false;
            return true;
        }
    }
}
