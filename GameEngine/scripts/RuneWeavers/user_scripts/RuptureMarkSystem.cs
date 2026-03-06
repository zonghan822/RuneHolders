using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Eno
{
    /*============================================================================================
    * RuptureMarkSystem — C# LogicSystem that manages the lifecycle of RuptureMark spell entities.
    *
    * SCENE SETUP:
    *   Attach this script to an empty scene node (no entity needed on the node itself).
    *   RuneCastingSystem calls RegisterMark() after SpawnRuptureMark succeeds.
    *
    * STATE MACHINE (mirrors the old C++ RuptureMarkState):
    *   SPAWNING  → after spawnDuration,  enters IDLE
    *   IDLE      → when a matching enemy enters detectionRadius, enters WINDUP
    *   WINDUP    → after windupDuration, enters EXPLODING and deals damage
    *   EXPLODING → after explosionDuration, enters FINISHED
    *   FINISHED  → entity is destroyed
    *
    * SPELL → TARGET ENEMY MAPPING:
    *   1 (ICE)   → entity names containing "Trailblight"
    *   2 (FIRE)  → entity names containing "NullFly"
    *   3 (LIGHT) → entity names containing "Bloodwing"
    *//*========================================================================================*/
    public class RuptureMarkSystem : LogicSystem
    {
        // ----------------------------------------------------------------
        // Configurable
        // ----------------------------------------------------------------
        public float detectionRadius   = 180.0f;
        public float damage            = 20.0f;
        public float spawnDuration     = 0.8f;
        public float windupDuration    = 0.5f;
        public float explosionDuration = 0.4f;

        // ----------------------------------------------------------------
        // Per-mark runtime data
        // ----------------------------------------------------------------
        private struct MarkData
        {
            public int   spellType;           // 1=ICE, 2=FIRE, 3=LIGHT
            public int   state;               // 0=SPAWNING 1=IDLE 2=WINDUP 3=EXPLODING 4=FINISHED
            public float timer;
            public uint  triggeredEntityID;   // enemy that stepped on the mark
        }

        // Static so RuneCastingSystem can call RegisterMark without a reference
        private static readonly Dictionary<uint, MarkData> s_activeMarks = new Dictionary<uint, MarkData>();

        // Buffer reused every frame to avoid GC pressure
        private static readonly uint[] s_nameQueryBuf = new uint[64];

        // ----------------------------------------------------------------
        // Called by RuneCastingSystem immediately after SpawnRuptureMark
        // ----------------------------------------------------------------
        public static void RegisterMark(uint markID, int spellType)
        {
            s_activeMarks[markID] = new MarkData
            {
                spellType         = spellType,
                state             = 0,   // SPAWNING
                timer             = 0f,
                triggeredEntityID = 0
            };
        }

        // ----------------------------------------------------------------
        // Begin — nothing to resolve (marks are registered dynamically)
        // ----------------------------------------------------------------
        public void Begin()
        {
            Console.WriteLine("[RuptureMarkSystem] Initialized.");
        }

        // ----------------------------------------------------------------
        // Update — tick every active mark's state machine
        // ----------------------------------------------------------------
        public void Update()
        {
            if (s_activeMarks.Count == 0) return;

            float dt = Bridge.GetDeltaTime();
            var toRemove = new List<uint>();

            foreach (uint markID in new List<uint>(s_activeMarks.Keys))
            {
                MarkData d = s_activeMarks[markID];
                d.timer += dt;

                // Validate entity still exists
                if (!Bridge.GetEntityPosition(markID, out float markX, out float markY))
                {
                    toRemove.Add(markID);
                    continue;
                }

                switch (d.state)
                {
                    case 0: // SPAWNING
                        if (d.timer >= spawnDuration)
                        {
                            d.state = 1;
                            d.timer = 0f;
                        }
                        break;

                    case 1: // IDLE — scan for enemy in range
                        string targetName = GetTargetName(d.spellType);
                        if (targetName.Length > 0)
                        {
                            int found = Bridge.FindAllEntitiesContainingName(targetName, s_nameQueryBuf, s_nameQueryBuf.Length);
                            for (int i = 0; i < found; i++)
                            {
                                uint enemyID = s_nameQueryBuf[i];
                                if (!Bridge.GetEntityPosition(enemyID, out float ex, out float ey)) continue;
                                float dist = Bridge.CalculateDistance(markX, markY, ex, ey);
                                if (dist <= detectionRadius)
                                {
                                    d.state             = 2; // WINDUP
                                    d.timer             = 0f;
                                    d.triggeredEntityID = enemyID;
                                    Bridge.SetEntityAnimation(markID, "windup");
                                    break;
                                }
                            }
                        }
                        break;

                    case 2: // WINDUP
                        if (d.timer >= windupDuration)
                        {
                            d.state = 3; // EXPLODING
                            d.timer = 0f;
                            Bridge.SetEntityAnimation(markID, "explosion");
                            if (d.triggeredEntityID != 0)
                                Bridge.AddEntityDamage(d.triggeredEntityID, damage);
                        }
                        break;

                    case 3: // EXPLODING
                        if (d.timer >= explosionDuration)
                            d.state = 4; // FINISHED
                        break;

                    case 4: // FINISHED
                        Bridge.DestroyEntity(markID);
                        toRemove.Add(markID);
                        break;
                }

                s_activeMarks[markID] = d;
            }

            foreach (uint id in toRemove)
                s_activeMarks.Remove(id);
        }

        // ----------------------------------------------------------------
        // Helper
        // ----------------------------------------------------------------
        private static string GetTargetName(int spellType)
        {
            switch (spellType)
            {
                case 1: return "Trailblight";
                case 2: return "NullFly";
                case 3: return "Bloodwing";
                default: return "";
            }
        }
    }
}
