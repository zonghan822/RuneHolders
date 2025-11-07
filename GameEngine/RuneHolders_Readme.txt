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

WASD → Move player (if spawned).
SPACEBAR → Player dash (if spawned).


---Player Transform---

1 → Scale Up
2 → Scale Down
3 → Rotate Clockwise
4 → Rotate Counter-clockwise


---Audio---

P / L → Select between two audio tracks
O → Play/Stop selected track

**** Stop audio with O before switching tracks ****


---Physics & Collision Testing ---

TESTING COLLISION DETECTION:

The demo includes test objects (pushable boxes) for collision testing:

Dynamic collision response with momentum transfer
Objects can push each other based on mass and velocity
Collision normal determine separation direction

Velocity: Current movement speed (X/Y)
Mass: Affects collision response (heavier = harder to push)
Drag: Air resistance/friction factor
Use Gravity: Toggle gravity for the entity
Is Kinematic: If true, object won't be pushed by physics

---Level Editor---

WASD → Move editor camera view
Play Button → Enter runtime/test mode (does NOT auto-save scene)
Stop Button → Exit runtime mode and reload scene from last save


---Scene Management---

Click scene name in Scene List → Switch to that scene
Create New Scene → Creates new scene with auto-incremented name
Rename Scene → Rename the active scene
Save Scene → Manually save current scene state
Delete Scene → Permanently delete active scene (with confirmation)

**** Stop button reloads the scene - any changes in Play mode are discarded ****
---------------------------
2.b Features
---------------------------

---Cursor System---

Cursor determines entity spawn location.
Adjustable speed via Main tab.


---Main Controls Tab---

PLAYER:
• Spawn Player → Creates player at indicator position with config from file
• Destroy Player → Removes player from scene
• Player stats loaded from player_config.txt

ENTITY SPAWNING:
• Spawn Entity → Creates entity at indicator position
• Entity settings: Width, Height, Shape (Rectangle/Circle), Color
• Optional colliders: None, Circle, or Rectangle
• "Show Colliders" toggle → Visualize collision boundaries

COLLIDER OPTIONS:
• Circle collider: Adjustable radius
• Rectangle collider: Adjustable width/height + adds PhysicsComponent (pushable)
• Entities with rectangle colliders can be pushed by player

STRESS TESTING:
• Spawn 2500 Entities → Creates 2500 random entities for performance testing
• Entities spawn at random positions with random sizes/colors


---Edit Tab---

ALL ENTITIES LIST:
• Shows total entity count
• Lists all entities with ID and name
• Click entity → Opens in Entity Inspector

DELETE FUNCTION:
• "Delete All Entities" button (except player)
• Useful for clearing test entities


---FPS Panel---

PERFORMANCE METRICS:
• Current FPS (frames per second)
• Frame time in milliseconds
• Per-system timing breakdown (Entities, Graphics, ImGui)
• Percentage of frame time per system

DEBUG BUTTONS:
• Show Entity Count → Displays total entities in scene
• Show Entity Characteristics → Lists all entity components


---Prefabs Tab---

CREATE PREFABS:
• Enter prefab name
• Set transform/size (Position X/Y, Width, Height)
• Choose shape (Rectangle, Circle)
• Choose color
• Toggle "Has Collider" option
• Click "Save New or Existing Prefabs" to save

LOAD PREFABS:
• Click "Load Existing Prefab" to scan saves directory
• Select prefab from dropdown list
• Selected prefab highlighted for placement

SPAWN PREFABS:
• Click "Spawn Prefab" → Spawns at default position
• OR Select prefab → Click in Game View to place at cursor position
• Click "Cancel Placement" to stop click-to-place mode

PREFAB FILES:
• Can edit existing prefabs by modifying settings and saving with same name

**** Loading an unsaved prefab name will crash to test crash logger ****
**** THIS IS INTENTIONAL BEHAVIOR FOR TESTING ****


---Scene Panel---

Located at top-left of editor interface

SCENE FILES:
• Displays current active scene name
• Lists all available scene files (.txt)
• Click scene name to switch scenes

SCENE OPERATIONS:
• Create new scene → Auto-incremented name (New Scene (1), New Scene (2), etc.)
• Rename Scene → Rename active scene file
• Save Scene → Manually save current state
• Delete Scene → Permanently remove scene (with confirmation)

SCENE LOADING:
• Application loads last scene from CurrentSceneSaved.txt on startup
• Switching scenes updates CurrentSceneSaved.txt automatically
• If CurrentSceneSaved.txt is deleted, creates new default scene

**** Creating multiple new scenes won't overwrite - names auto-increment ****


---Hierarchy Tab---

Spawn Player → Add player character to scene
Add Root Node → Create new parent entity (top-level)
Add Child Node → Create child entity under selected parent
Delete Selected Node → Remove entity and all children (recursive deletion)
Print Tree to Terminal → Debug output of full hierarchy structure
**** Deleting a parent deletes ALL children without individual confirmation ****


---Assets Browser---

Drag & Drop → Import textures (PNG, JPG, JPEG)
Click texture → Select for sprite sheet creation or entity assignment
Texture names → Use in DrawInfo component for entity rendering


---Entity Inspector---

Displays when entity is selected from Hierarchy or Edit tab
Components shown: Name, Transform, Physics, DrawInfo, Collision, Player
Drag controls → Adjust component values in real-time
All changes → Apply immediately to scene

**** Changes made during Play mode are temporary (lost on Stop) ****

----------------------------------------------------------
3. Team Roster
----------------------------------------------------------

RuneHolders

Qi Ming Jeremy CHIA - Project Manager ( Design Lead )
Sabrina Jing Jie LOH ( Audio Lead )
Muhammad Raziq BIN MOHD RAFIG - Technical Lead ( Rendering Champion )
Zong Han LAW ( Graphics Champion, Input Champion )
Caven KOH ( Physics Champion )
Zhe An LEE ( Engine Champion )
Yun Ji KOE ( Level Editor Champion )
Kimberly Charlotte Madlangbayan AMBROSIO ( Collision Champion )
