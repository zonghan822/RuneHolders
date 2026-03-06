using System;
using System.Runtime.InteropServices;

namespace Eno
{
    /*============================================================================================
    * RuneCastingSystem — C# LogicSystem that drives the rune-based spell casting mechanic.
    *
    * SCENE SETUP REQUIRED:
    *   Place 6 entities in the scene with the following names (must be in scene tree):
    *     "Rune_1", "Rune_2", "Rune_3", "Rune_4", "Rune_5", "Rune_6"
    *   Each entity needs: TransformComponent, DrawInfoComponent (white rune spritesheet,
    *   texID = runeNumber-1), VisibilityComponent (visible=true, z_index=15).
    *   Position them at (-9999, -9999) initially so they start off-screen.
    *
    * HOW IT WORKS:
    *   - Hold Left Mouse Button  → runes orbit the cursor in a circle
    *   - Drag over runes in sequence → draws connection lines between them
    *   - Release LMB or complete 4-rune sequence → spell fires or resets
    *
    * SPELL PATTERNS (rune numbers 1–6):
    *   ICE   : 3-2-5-4   (damages Trailblight enemies)
    *   FIRE  : 6-1-5-2   (damages NullFly enemies)
    *   LIGHT : 4-1-3-6   (damages Bloodwing enemies)
    *//*========================================================================================*/
    public class RuneCastingSystem : ScriptBase
    {
        // ----------------------------------------------------------------
        // Configurable parameters (editable in editor via [Port] if needed)
        // ----------------------------------------------------------------
        public float circleRadius   = 100.0f;   // Radius of rune circle around cursor
        public float hoverRadius    = 30.0f;    // Distance threshold to "select" a rune
        public float runeSizeLarge  = 56.0f;    // Draw scale for available/selected runes
        public float runeSizeSmall  = 40.0f;    // Draw scale for unavailable/invalid runes
        public float lineWidth      = 8.0f;     // Thickness of connection lines

        // ----------------------------------------------------------------
        // Spell patterns — rune numbers 1-6 in connection order
        // ----------------------------------------------------------------
        private static readonly int[] ICE_PATTERN   = { 3, 2, 5, 4 };
        private static readonly int[] FIRE_PATTERN  = { 6, 1, 5, 2 };
        private static readonly int[] LIGHT_PATTERN = { 4, 1, 3, 6 };

        // ----------------------------------------------------------------
        // Rune entity IDs (sceneObjectIDs from FindEntityByName)
        // positionSlot[i] → runeNumber[i]: the rune entity at circle position i
        // ----------------------------------------------------------------
        private uint[] runeEntityIDs = new uint[6];

        // Rune number at each circle position slot (clockwise from top-left)
        // Matches the order used in initializeRuneSystem: positions 0-5 → rune numbers 6,1,2,3,4,5
        private readonly int[] slotToRuneNumber = { 6, 1, 2, 3, 4, 5 };

        // ----------------------------------------------------------------
        // Runtime state
        // ----------------------------------------------------------------
        private bool runesActive    = false;
        private bool spellCasted    = false;
        private bool isDrawingLine  = false;

        private bool[] slotConnected = new bool[6];     // which slot has been selected

        // Connection sequence: up to 4 rune numbers in order
        private int[]   connectedSequence = new int[4];
        private float[] connectedX        = new float[4];
        private float[] connectedY        = new float[4];
        private int     connectionCount   = 0;

        // Circle centre at the time runes were activated (for spell spawn position)
        private float circleCenterX = 0f;
        private float circleCenterY = 0f;

        // Line color: 0 = yellow (selecting), 1 = green (success), 2 = red (invalid)
        private int lineColorState = 0;

        // Feedback timers
        private bool  isInvalidState  = false;
        private float invalidTimer    = 0f;
        private const float INVALID_DURATION = 1.0f;

        private bool  isSuccessState  = false;
        private float successTimer    = 0f;
        private const float SUCCESS_DURATION = 0.5f;

        // ----------------------------------------------------------------
        // Begin — resolve entity IDs once
        // ----------------------------------------------------------------
        public void Begin()
        {
            string[] runeNames = { "Rune_6", "Rune_1", "Rune_2", "Rune_3", "Rune_4", "Rune_5" };
            for (int i = 0; i < 6; i++)
            {
                runeEntityIDs[i] = Bridge.FindEntityByName(runeNames[i]);
                if (runeEntityIDs[i] == 0)
                    Console.WriteLine($"[RuneCastingSystem] WARNING: Could not find entity '{runeNames[i]}' in scene.");
            }
            Console.WriteLine("[RuneCastingSystem] Initialized.");
        }

        // ----------------------------------------------------------------
        // Update — runs every frame
        // ----------------------------------------------------------------
        public void Update()
        {
            // Always clear last frame's line queue first
            Bridge.ClearRuneLines();

            float dt = Bridge.GetDeltaTime();

            // --- Invalid feedback state (red flash, then reset) ---
            if (isInvalidState)
            {
                invalidTimer += dt;
                DrawLines(-1f, -1f); // draw current lines in red
                if (invalidTimer >= INVALID_DURATION)
                {
                    DeactivateRunes();
                    isInvalidState = false;
                    invalidTimer   = 0f;
                }
                return;
            }

            // --- Success feedback state (green flash, then reset) ---
            if (isSuccessState)
            {
                successTimer += dt;
                DrawLines(-1f, -1f); // draw current lines in green
                if (successTimer >= SUCCESS_DURATION)
                {
                    DeactivateRunes();
                    isSuccessState = false;
                    successTimer   = 0f;
                }
                return;
            }

            // --- Normal input ---
            Bridge.GetMouseWorldPosition(out float mouseX, out float mouseY);

            // LMB pressed this frame → activate runes
            if (Bridge.IsMousePressed(0) && !runesActive)
            {
                ActivateRunes(mouseX, mouseY);
            }

            // LMB held → update hover detection and draw trailing line
            if (Bridge.IsMouseHeld(0) && runesActive)
            {
                UpdateHover(mouseX, mouseY);

                if (isDrawingLine)
                    DrawLines(mouseX, mouseY);
            }

            // LMB released → hide runes
            if (Bridge.IsMouseReleased(0) && runesActive)
            {
                DeactivateRunes();
            }
        }

        // ----------------------------------------------------------------
        // ActivateRunes — move rune entities into circle around cursor
        // ----------------------------------------------------------------
        private void ActivateRunes(float centerX, float centerY)
        {
            runesActive      = true;
            circleCenterX    = centerX;
            circleCenterY    = centerY;
            connectionCount  = 0;
            isDrawingLine    = false;
            spellCasted      = false;
            lineColorState   = 0;

            for (int i = 0; i < 6; i++)
                slotConnected[i] = false;

            for (int i = 0; i < 4; i++)
                connectedSequence[i] = -1;

            // Position each rune in a clockwise circle starting from top-left (-120°)
            for (int i = 0; i < 6; i++)
            {
                if (runeEntityIDs[i] == 0) continue;

                double angleDeg = -120.0 + i * 60.0;
                double angleRad = angleDeg * Math.PI / 180.0;
                float  rx       = centerX + circleRadius * (float)Math.Cos(angleRad);
                float  ry       = centerY + circleRadius * (float)Math.Sin(angleRad);

                Bridge.SetEntityPosition(runeEntityIDs[i], rx, ry);
            }

            UpdateVisualStates();
            Bridge.PlayInstantAudio("SFX", $"Rune-GalaxyBellPad-{new Random().Next(1, 7)}");
        }

        // ----------------------------------------------------------------
        // DeactivateRunes — move all runes back off-screen
        // ----------------------------------------------------------------
        private void DeactivateRunes()
        {
            for (int i = 0; i < 6; i++)
            {
                if (runeEntityIDs[i] != 0)
                    Bridge.SetEntityPosition(runeEntityIDs[i], -9999f, -9999f);
            }

            runesActive     = false;
            isDrawingLine   = false;
            spellCasted     = false;
            connectionCount = 0;
            lineColorState  = 0;

            for (int i = 0; i < 4; i++) connectedSequence[i] = -1;
            for (int i = 0; i < 6; i++) slotConnected[i]     = false;
        }

        // ----------------------------------------------------------------
        // UpdateHover — detect if mouse entered a rune's radius; add to sequence
        // ----------------------------------------------------------------
        private void UpdateHover(float mouseX, float mouseY)
        {
            for (int i = 0; i < 6; i++)
            {
                if (runeEntityIDs[i] == 0 || slotConnected[i] || spellCasted) continue;

                if (!Bridge.GetEntityPosition(runeEntityIDs[i], out float rx, out float ry)) continue;

                float dist = (float)Math.Sqrt((mouseX - rx) * (mouseX - rx) +
                                              (mouseY - ry) * (mouseY - ry));
                if (dist > hoverRadius) continue;

                int runeNum = slotToRuneNumber[i];

                // Check validity
                bool valid = IsValidNextRune(runeNum);
                if (!valid)
                {
                    // Only trigger invalid state if at least one rune already connected
                    if (connectionCount > 0)
                    {
                        lineColorState = 2; // red
                        isInvalidState = true;
                        invalidTimer   = 0f;
                        UpdateVisualStates();
                        Bridge.PlayInstantAudio("SFX", "Rune-GalaxyBellPad-3");
                    }
                    continue;
                }

                // Add rune to sequence
                connectedSequence[connectionCount] = runeNum;
                connectedX[connectionCount]        = rx;
                connectedY[connectionCount]        = ry;
                connectionCount++;
                slotConnected[i] = true;
                isDrawingLine    = true;

                UpdateVisualStates();
                Bridge.PlayInstantAudio("SFX", $"Rune-GalaxyBellPad-{new Random().Next(1, 7)}");

                // Check for completed 4-rune sequence
                if (connectionCount == 4)
                {
                    int spellType = CheckSpellPattern();
                    if (spellType != 0)
                    {
                        // Valid spell — green flash and spawn
                        lineColorState = 1; // green
                        isSuccessState = true;
                        successTimer   = 0f;
                        uint markID = Bridge.SpawnRuptureMark(circleCenterX, circleCenterY, spellType);
                        if (markID != 0)
                            RuptureMarkSystem.RegisterMark(markID, spellType);
                        Bridge.PlayInstantAudio("SFX", "Rune-GalaxyBellPad-5");
                    }
                    else
                    {
                        // Invalid 4-rune combo — red flash
                        lineColorState = 2; // red
                        isInvalidState = true;
                        invalidTimer   = 0f;
                        Bridge.PlayInstantAudio("SFX", "Rune-GalaxyBellPad-3");
                    }
                    spellCasted = true;
                    return;
                }

                break; // One new connection per frame
            }
        }

        // ----------------------------------------------------------------
        // DrawLines — queue line segments for the C++ render phase
        //   lineEndX/Y: current mouse position for the trailing segment (-1 to skip)
        // ----------------------------------------------------------------
        private void DrawLines(float lineEndX, float lineEndY)
        {
            if (connectionCount == 0) return;

            float r, g, b;
            switch (lineColorState)
            {
                case 1:  r = 0f; g = 1f; b = 0f; break; // green
                case 2:  r = 1f; g = 0f; b = 0f; break; // red
                default: r = 1f; g = 1f; b = 0f; break; // yellow
            }

            // Draw segments between connected runes
            for (int i = 0; i < connectionCount - 1; i++)
            {
                Bridge.AddRuneLine(connectedX[i], connectedY[i],
                                   connectedX[i + 1], connectedY[i + 1],
                                   r, g, b, 0.9f, lineWidth);
            }

            // Draw trailing segment from last connected rune to current mouse position
            if (!spellCasted && lineEndX >= 0f && connectionCount > 0)
            {
                Bridge.AddRuneLine(connectedX[connectionCount - 1], connectedY[connectionCount - 1],
                                   lineEndX, lineEndY,
                                   r, g, b, 0.9f, lineWidth);
            }
        }

        // ----------------------------------------------------------------
        // UpdateVisualStates — tint and scale each rune based on current state
        // ----------------------------------------------------------------
        private void UpdateVisualStates()
        {
            int validMask = GetValidNextRuneMask();
            float largeScale = runeSizeLarge / 56.0f;
            float smallScale = runeSizeSmall / 56.0f;

            for (int i = 0; i < 6; i++)
            {
                if (runeEntityIDs[i] == 0) continue;
                int runeNum = slotToRuneNumber[i];

                if (isInvalidState)
                {
                    // All runes red and small
                    Bridge.SetEntityScale(runeEntityIDs[i], smallScale, smallScale);
                    Bridge.SetEntityDrawColor(runeEntityIDs[i], 1f, 0f, 0f, 1f);
                }
                else if (slotConnected[i])
                {
                    // Selected/connected — large green
                    Bridge.SetEntityScale(runeEntityIDs[i], largeScale, largeScale);
                    Bridge.SetEntityDrawColor(runeEntityIDs[i], 0f, 1f, 0f, 1f);
                }
                else if ((validMask & (1 << runeNum)) != 0)
                {
                    // Next valid choice — large white
                    Bridge.SetEntityScale(runeEntityIDs[i], largeScale, largeScale);
                    Bridge.SetEntityDrawColor(runeEntityIDs[i], 1f, 1f, 1f, 1f);
                }
                else
                {
                    // Unavailable — small grey
                    Bridge.SetEntityScale(runeEntityIDs[i], smallScale, smallScale);
                    Bridge.SetEntityDrawColor(runeEntityIDs[i], 0.4f, 0.4f, 0.4f, 1f);
                }
            }
        }

        // ----------------------------------------------------------------
        // IsValidNextRune — check if runeNum is a legal next selection
        // ----------------------------------------------------------------
        private bool IsValidNextRune(int runeNum)
        {
            // No runes yet: only starting runes 3, 4, 6 are valid
            if (connectionCount == 0)
                return runeNum == 3 || runeNum == 4 || runeNum == 6;

            // Check if runeNum matches the next position in any still-viable pattern
            bool iceOk = true, fireOk = true, lightOk = true;
            for (int i = 0; i < connectionCount; i++)
            {
                if (connectedSequence[i] != ICE_PATTERN[i])   iceOk   = false;
                if (connectedSequence[i] != FIRE_PATTERN[i])  fireOk  = false;
                if (connectedSequence[i] != LIGHT_PATTERN[i]) lightOk = false;
            }

            if (connectionCount < 4)
            {
                if (iceOk   && ICE_PATTERN[connectionCount]   == runeNum) return true;
                if (fireOk  && FIRE_PATTERN[connectionCount]  == runeNum) return true;
                if (lightOk && LIGHT_PATTERN[connectionCount] == runeNum) return true;
            }

            return false;
        }

        // ----------------------------------------------------------------
        // CheckSpellPattern — return spell type if sequence matches, else 0
        // ----------------------------------------------------------------
        private int CheckSpellPattern()
        {
            if (connectionCount != 4) return 0;

            bool ice = true, fire = true, light = true;
            for (int i = 0; i < 4; i++)
            {
                if (connectedSequence[i] != ICE_PATTERN[i])   ice   = false;
                if (connectedSequence[i] != FIRE_PATTERN[i])  fire  = false;
                if (connectedSequence[i] != LIGHT_PATTERN[i]) light = false;
            }

            if (ice)   return 1; // SpellType::ICE
            if (fire)  return 2; // SpellType::FIRE
            if (light) return 3; // SpellType::LIGHT
            return 0;
        }

        // ----------------------------------------------------------------
        // GetValidNextRuneMask — bitmask of rune numbers valid as next pick
        // ----------------------------------------------------------------
        private int GetValidNextRuneMask()
        {
            int mask = 0;
            if (connectionCount == 0)
            {
                mask |= (1 << 3);
                mask |= (1 << 4);
                mask |= (1 << 6);
                return mask;
            }

            bool iceOk = true, fireOk = true, lightOk = true;
            for (int i = 0; i < connectionCount; i++)
            {
                if (connectedSequence[i] != ICE_PATTERN[i])   iceOk   = false;
                if (connectedSequence[i] != FIRE_PATTERN[i])  fireOk  = false;
                if (connectedSequence[i] != LIGHT_PATTERN[i]) lightOk = false;
            }

            if (connectionCount < 4)
            {
                if (iceOk)   mask |= (1 << ICE_PATTERN[connectionCount]);
                if (fireOk)  mask |= (1 << FIRE_PATTERN[connectionCount]);
                if (lightOk) mask |= (1 << LIGHT_PATTERN[connectionCount]);
            }

            return mask;
        }
    }
}
