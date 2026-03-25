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

        // ── Runtime state ────────────────────────────────────────────────────────
        private bool        _active         = false;
        private float       _originX        = 0f;
        private float       _originY        = 0f;

        // Rune ring
        private float[]     _runeX          = new float[RUNE_COUNT];
        private float[]     _runeY          = new float[RUNE_COUNT];
        private Sprite2D?[] _runes          = new Sprite2D?[RUNE_COUNT];
        private string[]    _runeStates     = new string[RUNE_COUNT];
        private List<int>   _connected      = new();

        // Casting visuals
        private Sprite2D?   _spellBase      = null;   // ring at origin
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

        // ── Script callbacks ─────────────────────────────────────────────────────
        public void Begin()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
            {
                _runes[i]      = null;
                _runeStates[i] = "Disabled";
            }
        }

        public void Update()
        {
            if (Scroll_Dialouge_Script.DialogueActive)
            {
                if (_active) Cancel();
                return;
            }

            if (!_active)
            {
                if (Input.isMousePressed(0))
                    Activate();
            }
            else
            {
                if (Input.isMouseHeld(0))
                {
                    CheckConnections();
                    UpdateTrailLine(Input.mouseWorldPosition.x, Input.mouseWorldPosition.y);
                }
                else
                    Release();

                if (Input.isMousePressed(1))
                    Cancel();
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

            SetRuneState(runeIndex, "Nullified");
            RefreshReadyStates();
        }

        // ── Private helpers ──────────────────────────────────────────────────────

        private void Activate()
        {
            _originX       = Input.mouseWorldPosition.x;
            _originY       = Input.mouseWorldPosition.y;
            _activePattern = -1;
            _connected.Clear();

            _spellBase = SpawnPrefab("SpellBase", _originX, _originY) as Sprite2D;

            SpawnRunes();
            _active = true;

            // Apply rune bans from any visible Nullfly enemies
            foreach (int runeIdx in EnemyColorRegistry.ActiveBans)
                EnemyNullifyRune(runeIdx);

            // Slow world time and freeze all enemies
            Time.timeScale = slowTimeScale;
            Bridge.SetGroupLowPass("SFX", 600f);
            foreach (Enemy enemy in GetAllEnemies())
                enemy.changeState("Frozen");

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
            }
        }

        private void CheckConnections()
        {
            float mx     = Input.mouseWorldPosition.x;
            float my     = Input.mouseWorldPosition.y;
            float snapSq = SNAP_RADIUS * SNAP_RADIUS;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runeStates[i] != "Ready") continue;

                float dx = mx - _runeX[i];
                float dy = my - _runeY[i];
                if (dx * dx + dy * dy > snapSq) continue;

                _connected.Add(i);
                SetRuneState(i, "Activated");
                try { GetNode<AudioNode>("Rune-GalaxyBellPad-1")?.play(); } catch { }

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

                // If the full pattern is now complete, cast immediately
                if (_activePattern >= 0 && PatternMatches(_connected, Patterns[_activePattern]))
                {
                    CastSpell(_activePattern);
                    return;
                }

                RefreshReadyStates();
            }
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
            _active = false;
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            SpawnPrefab(SpellPrefabs[spellIndex], _originX, _originY);
            EndSlowTime(attack: true);
        }

        private void Release()
        {
            _active = false;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            EndSlowTime();
        }

        private void Cancel()
        {
            _active = false;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnLines();
            DespawnRunes();
            EndSlowTime();
        }

        // Restore normal time, unfreeze enemies, and trigger StopWatch fade-out.
        // attack: true when called from CastSpell — sets player to "Attack"; false restores Idle/Walk.
        private void EndSlowTime(bool attack = false)
        {
            Time.timeScale = 1.0f;
            Bridge.SetGroupLowPass("SFX", 0f);
            foreach (Enemy enemy in GetAllEnemies())
                enemy.changeState("Idle");
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
            return Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.S) ||
                   Input.isKeyHeld(KeyCode.A) || Input.isKeyHeld(KeyCode.D);
        }

        // ── Pattern helpers ──────────────────────────────────────────────────────

        // Returns the index of the single remaining viable pattern, or -1 if ambiguous / none.
        private int GetViablePatternIndex()
        {
            int found = -1;
            int depth = _connected.Count;

            for (int s = 0; s < Patterns.Length; s++)
            {
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
                if (_runes[i] != null)
                {
                    FreeNode(_runes[i]);
                    _runes[i]      = null;
                    _runeStates[i] = "Disabled";
                }
            }
            _connected.Clear();
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
