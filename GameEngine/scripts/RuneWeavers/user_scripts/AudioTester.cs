using System;

namespace Eno
{
    // Attach this script to any entity that has an Audio component in the editor.
    // Use the keybinds below to manually fire the same audio calls that the door
    // scripts make, without needing a player entity in the scene.
    //
    // Key bindings (all single-press):
    //   P  - PlayEntityAudio(this.ID)       — same as DoorScript trigger
    //   S  - StopEntityAudio(this.ID)       — stop whatever is playing on this entity
    //   I  - PlayInstantAudio(group, sound) — same as Door2Script trigger, uses the
    //                                         group/sound names set in the Port fields
    //   R  - Reset "hasPlayed" flag so P / I can fire again without restarting the scene
    public class AudioTester : ScriptBase
    {
        // ---- Configurable via the editor Ports ----

        // Group name passed to PlayInstantAudio (matches your audio group in the editor)
        [Port("Instant audio group name")]
        public string instantGroup = "SFX";

        // Sound name passed to PlayInstantAudio (matches a sound inside that group)
        [Port("Instant audio sound name")]
        public string instantSound = "SpellCast_01";

        // ---- Internal state ----
        private bool _entityPlayed = false;
        private bool _instantPlayed = false;

        public void Begin()
        {
            Console.WriteLine("[AudioTester] Ready.");
            Console.WriteLine("[AudioTester]  P = PlayEntityAudio on this entity");
            Console.WriteLine("[AudioTester]  S = StopEntityAudio on this entity");
            Console.WriteLine($"[AudioTester]  I = PlayInstantAudio(\"{instantGroup}\", \"{instantSound}\")");
            Console.WriteLine("[AudioTester]  R = Reset flags so P / I can fire again");
        }

        public void Update()
        {
            // P — play the audio component attached to this entity (mirrors DoorScript)
            if (Input.IsKeyPressed(KeyCode.P) && !_entityPlayed)
            {
                bool ok = Bridge.PlayEntityAudio(this.ID);
                Console.WriteLine($"[AudioTester] PlayEntityAudio({this.ID}) -> {ok}");
                _entityPlayed = true;
            }

            // S — stop the audio component on this entity
            if (Input.IsKeyPressed(KeyCode.S))
            {
                bool ok = Bridge.StopEntityAudio(this.ID);
                Console.WriteLine($"[AudioTester] StopEntityAudio({this.ID}) -> {ok}");
                // Also clear the played flag so P works again immediately after stopping
                _entityPlayed = false;
            }

            // I — fire a one-shot instant audio call (mirrors Door2Script)
            if (Input.IsKeyPressed(KeyCode.I) && !_instantPlayed)
            {
                Bridge.PlayInstantAudio(instantGroup, instantSound);
                Console.WriteLine($"[AudioTester] PlayInstantAudio(\"{instantGroup}\", \"{instantSound}\")");
                _instantPlayed = true;
            }

            // R — reset both played flags so you can re-trigger without reloading the scene
            if (Input.IsKeyPressed(KeyCode.R))
            {
                _entityPlayed = false;
                _instantPlayed = false;
                Console.WriteLine("[AudioTester] Flags reset — P and I are live again");
            }
        }
    }
}
