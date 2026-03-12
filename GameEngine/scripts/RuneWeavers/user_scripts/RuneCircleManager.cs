/*============================================================================================
* @file         RuneCircleManager.cs
* @brief        Handles the rune circle spell-casting mechanic.
*
*               When the player clicks and holds the left mouse button, six rune nodes
*               appear in a circle around the cursor. The player drags through them in a
*               specific order to form a pattern. On release, the pattern is matched against
*               the three known spells and, if valid, the corresponding spell prefab is
*               spawned at the circle's origin.
*
* Rune layout (clockwise from top):
*
*          5 0
*        4     1
*          3 2
*
* Spell patterns:
*   RED  – 0 → 1 → 2 → 3
*   YELLOW – 5 → 4 → 1 → 2
*   BLUE – 4 → 3 → 5 → 0
*
* Required prefabs (create in the Prefab Editor before use):
*   "Rune"       – sprite shown at each rune position;
*                  needs states "idle" and "activated"
*   "RedSpell"  – spawned on RED pattern
*   "YellowSpell" – spawned on YELLOW pattern
*   "BlueSpell" – spawned on BLUE pattern
*============================================================================================*/

using System;
using System.Collections.Generic;

namespace Eno
{
    public class RuneCircleManager : ScriptBase
    {
        // ── Tuneable values ──────────────────────────────────────────────────────
        private const float RUNE_RING_RADIUS = 80f;   // world-unit radius of the rune ring
        private const float SNAP_RADIUS      = 28f;   // how close the cursor must be to connect a rune
        private const int   RUNE_COUNT       = 6;

        // ── Spell patterns ───────────────────────────────────────────────────────
        // Each inner array is the exact ordered sequence of rune indices the player
        // must connect to cast that spell.
        private static readonly int[][] Patterns = new int[][]
        {
            new int[] { 0, 1, 2, 3 },    // RED    – 0 → 1 → 2 → 3
            new int[] { 5, 4, 1, 2 },    // YELLOW – 5 → 4 → 1 → 2
            new int[] { 4, 3, 5, 0 },    // BLUE   – 4 → 3 → 5 → 0
        };

        private static readonly string[] SpellPrefabs = new string[]
        {
            "RedSpell",
            "YellowSpell",
            "BlueSpell",
        };

        // ── Runtime state ────────────────────────────────────────────────────────
        private bool      _active    = false;
        private float     _originX   = 0f;
        private float     _originY   = 0f;

        // World positions of the 6 rune nodes
        private float[]   _runeX     = new float[RUNE_COUNT];
        private float[]   _runeY     = new float[RUNE_COUNT];

        // Spawned rune nodes (null = not spawned)
        private Node?[]   _runes     = new Node?[RUNE_COUNT];

        // Sequence of rune indices connected so far this cast
        private List<int> _connected = new();

        // ── Script callbacks ─────────────────────────────────────────────────────
        public void Begin()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
                _runes[i] = null;
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

                // Right-click cancels without casting
                if (Input.isMousePressed(1))
                    Cancel();
            }
        }

        // ── Private helpers ──────────────────────────────────────────────────────

        private void Activate()
        {
            _originX = Input.mouseWorldPosition.x;
            _originY = Input.mouseWorldPosition.y;
            _connected.Clear();
            SpawnRunes();
            _active = true;
        }

        private void SpawnRunes()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
            {
                // Rune 0 is at the top (90°); each successive rune is 60° clockwise.
                float angleDeg = 90f - i * (360f / RUNE_COUNT);
                float angleRad = angleDeg * MathF.PI / 180f;

                _runeX[i] = _originX + RUNE_RING_RADIUS * MathF.Cos(angleRad);
                _runeY[i] = _originY + RUNE_RING_RADIUS * MathF.Sin(angleRad);

                _runes[i] = SpawnPrefab("Rune", _runeX[i], _runeY[i]);
            }
        }

        private void CheckConnections()
        {
            float mx     = Input.mouseWorldPosition.x;
            float my     = Input.mouseWorldPosition.y;
            float snapSq = SNAP_RADIUS * SNAP_RADIUS;

            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_connected.Contains(i)) continue;

                float dx = mx - _runeX[i];
                float dy = my - _runeY[i];

                if (dx * dx + dy * dy <= snapSq)
                {
                    _connected.Add(i);
                    ActivateRune(i);
                }
            }
        }

        // Transition the rune to its "activated" state to give visual feedback.
        private void ActivateRune(int index)
        {
            (_runes[index] as Sprite2D)?.changeState("Activated");
        }

        private void Release()
        {
            _active = false;

            // Check each spell pattern in priority order
            for (int s = 0; s < Patterns.Length; s++)
            {
                if (PatternMatches(_connected, Patterns[s]))
                {
                    SpawnPrefab(SpellPrefabs[s], _originX, _originY);
                    break;
                }
            }

            DespawnRunes();
        }

        private void Cancel()
        {
            _active = false;
            DespawnRunes();
        }

        private void DespawnRunes()
        {
            for (int i = 0; i < RUNE_COUNT; i++)
            {
                if (_runes[i] != null)
                {
                    FreeNode(_runes[i]);
                    _runes[i] = null;
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
