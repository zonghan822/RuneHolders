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
* Rune layout (clockwise from top):
*
*          5 0
*        4     1
*          3 2
*
* Spell patterns:
*   RED    – 0 → 1 → 2 → 3
*   YELLOW – 5 → 4 → 1 → 2
*   BLUE   – 4 → 3 → 5 → 0
*
* Rune states:
*   Disabled  – not a valid next step in any remaining pattern
*   Ready     – a valid next connection in at least one remaining pattern
*   Activated – already connected by the player this cast
*   Nullified – blocked by an enemy; patterns requiring this rune are eliminated
*
* Required prefabs:
*   "Rune"              – 6-column spritesheet; UV column i selects the rune glyph.
*                         States: Disabled | Ready | Activated | Nullified
*   "SpellBase"         – ring / aura shown at origin while casting
*   "RedSpellCasting"   – charge indicator for RED; spritesheet has SPELL_STEPS columns
*   "YellowSpellCasting"– charge indicator for YELLOW
*   "BlueSpellCasting"  – charge indicator for BLUE
*   "RedSpell"          – final spell; attach SpellEffect.cs; states: windup | explosion
*   "YellowSpell"       – final spell
*   "BlueSpell"         – final spell
*============================================================================================*/

using System;
using System.Collections.Generic;

namespace Eno
{
    public class RuneSystem : ScriptBase
    {
        // ── Tuneable values ──────────────────────────────────────────────────────
        private const float RUNE_RING_RADIUS = 80f;
        private const float SNAP_RADIUS      = 28f;
        private const int   RUNE_COUNT       = 6;

        // ── Spell definitions ────────────────────────────────────────────────────
        private static readonly int[][] Patterns = new int[][]
        {
            new int[] { 0, 1, 2, 3 },    // RED    – 0 → 1 → 2 → 3
            new int[] { 5, 4, 1, 2 },    // YELLOW – 5 → 4 → 1 → 2
            new int[] { 4, 3, 5, 0 },    // BLUE   – 4 → 3 → 5 → 0
        };

        // Spritesheet columns per SpellCasting prefab = Patterns[s].Length
        private static readonly string[] SpellCastingPrefabs = new string[]
        {
            "RedSpellCasting",
            "YellowSpellCasting",
            "BlueSpellCasting",
        };

        private static readonly string[] SpellPrefabs = new string[]
        {
            "RedSpell",
            "YellowSpell",
            "BlueSpell",
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
            if (!_active)
            {
                if (Input.isMousePressed(0))
                    Activate();
            }
            else
            {
                if (Input.isMouseHeld(0))
                    CheckConnections();
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
            if (_runeStates[runeIndex] == "Activated") return;
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
        }

        private void SpawnRunes()
        {
            HashSet<int> nextValid = GetNextValidRunes();

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                float angleDeg = 90f - i * (360f / RUNE_COUNT);
                float angleRad = angleDeg * MathF.PI / 180f;

                _runeX[i] = _originX + RUNE_RING_RADIUS * MathF.Cos(angleRad);
                _runeY[i] = _originY + RUNE_RING_RADIUS * MathF.Sin(angleRad);

                _runes[i] = SpawnPrefab("Rune", _runeX[i], _runeY[i]) as Sprite2D;
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

                // Determine active pattern on first connection (or when it narrows to one)
                TryDeterminePattern();

                // Advance the charge visualisation
                UpdateSpellCastingUV();

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

        // Updates the SpellCasting sprite UV to show the current charge level.
        // The spritesheet has Patterns[_activePattern].Length columns, one per connection step.
        private void UpdateSpellCastingUV()
        {
            if (_spellCasting == null || _activePattern < 0) return;

            int   steps = Patterns[_activePattern].Length;
            float uvW   = 1.0f / steps;
            // Frame index = connections made so far minus 1 (first connection = frame 0)
            int   frame = Math.Max(0, _connected.Count - 1);
            _spellCasting.setUV(new Vector2(frame * uvW, 0.0f), new Vector2((frame + 1) * uvW, 1.0f));
        }

        private void CastSpell(int spellIndex)
        {
            _active = false;
            DespawnCastingVisuals();
            DespawnRunes();
            SpawnPrefab(SpellPrefabs[spellIndex], _originX, _originY);
        }

        private void Release()
        {
            _active = false;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnRunes();
        }

        private void Cancel()
        {
            _active = false;
            NullifyUnconnected();
            DespawnCastingVisuals();
            DespawnRunes();
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
