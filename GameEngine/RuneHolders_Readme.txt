==========================================================
RuneHolders README
==========================================================

RuneWeaver is an experimental spellcasting boss-fighting game where players weave runes
directly on the screen to unleash powerful magic. Spells can be chained together for unique 
and unexpected effects, allowing players to creatively strategize in battles against formidable bosses.

----------------------------------------------------------
1. Game Concept
----------------------------------------------------------

Draw runes to cast spells.

Chain spells together to create new effects.

Experiment with different combinations to find devastating synergies.

Fight bosses that challenge your creativity and reflexes.

The goal is to empower the player with freedom of expression in combat, blending artful gesture 
input with tactical spellcasting.

----------------------------------------------------------
2. Current State of Demo ( Inputs and Usage )
----------------------------------------------------------
---------------------------
2.a Controls
---------------------------

---General---

Arrow Keys → Move cursor (red square).

WASD → Move player (if spawned).

Spacebar → Player Dash (if spawned).

---Player Transform---

1 → Scale Up

2 → Scale Down

3 → Rotate Clockwise

4 → Rotate Counter-clockwise

---Audio---

P / L → Select between two audio tracks

O → Play/Stop selected track

**** Stop audio with O before switching tracks ****

---Physics---

***Physics has to be tested at the start before collision***

M → Toggle keyframe mode

N → Move to next frame

---------------------------
2.b Features
---------------------------

---Cursor System---

Cursor determines entity spawn location.

Adjustable speed via Main tab.

---Main Tab---

Spawn/Despawn player entity.

Spawn square entities and edit their width, height, and color.

Add circle or square colliders (toggle visibility, adjust parameters).

Spawn 2500 entities at once for stress testing.

---Edit Tab---

Delete all entities except the player.

---FPS Tab---

Displays FPS and per-system timings.

---Prefab Tab---

Save current entity settings with a custom name.

Load and spawn saved prefabs.

**** Loading an unsaved name will purposefully crash the application to test the crash logger. ****
**** THIS IS A FEATURE NOT A BUG ****

----------------------------------------------------------
3. Team Roster
----------------------------------------------------------

RuneHolders

CHIA Qi Ming Jeremy - Project Manager ( Design Lead )
LOH Sabrina Jing Jie ( Audio Lead )
Muhammad Raziq Bin Mohd Rafig - Technical Lead ( Rendering Champion )
LAW Zong Han ( Graphics Champion, Input Champion )
KOH Caven ( Physics Champion )
LEE Zhe An ( Engine Champion )
KOE YunJi ( Level Editor Champion )
AMBROSIO Kimberly Charlotte Madlangbayan ( Collision Champion )
