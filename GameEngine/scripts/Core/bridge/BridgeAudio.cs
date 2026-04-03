using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Audio Component  (implemented in eno_bridge_audio.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioPlay(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioStop(uint sceneObjectID);

        // Returns 1 if the audio channel is currently playing, 0 if it has finished or never started.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int IsAudioPlaying(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetAudioVolume(uint sceneObjectID);

        // volume: 0 – 100
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioVolume(uint sceneObjectID, float volume);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetAudioPitch(uint sceneObjectID);

        // pitch: 1.0 = normal
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioPitch(uint sceneObjectID, float pitch);

        // Enable/disable spatial audio (distance attenuation + panning) for the audio node
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioSpatial(uint sceneObjectID, int spatial);

        // distance: world units at which volume reaches zero
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioMaxDistance(uint sceneObjectID, float distance);

        // Pause/resume all audio channel groups (BGM + SFX) — used by in-game pause
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioPaused([MarshalAs(UnmanagedType.I1)] bool paused);

        // Pause/resume a single channel group by name ("BGM" or "SFX")
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetGroupPaused(string groupName, [MarshalAs(UnmanagedType.I1)] bool paused);

        // Change the channel group an audio node routes through ("BGM" or "SFX")
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioGroup(uint sceneObjectID, string groupName);

        // groupName: "BGM" or "SFX", volume: 0 – 100
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetGroupVolume(string groupName, float volume);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern float GetGroupVolume(string groupName);

        // Master channel group volume: 0 – 100
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetMasterVolume(float volume);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetMasterVolume();

        // Persist all three volumes to config.txt so they survive scene changes and game restarts
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SaveAudioVolumes(float master, float bgm, float sfx);

        // Load one volume by config key ("Audio Master", "Audio BGM", "Audio SFX"); returns defaultVal if absent
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern float LoadAudioVolume(string key, float defaultVal);

        // Play a sound
        // groupName: "SFX" or "BGM", soundName: asset name e.g. "Typing_01"
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioPlayInstant(uint ID, float x, float y);

        // Play a looping sound by name. Call AudioStopInstant to stop it.
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioPlayInstantLoop(string groupName, string soundName);

        // Stop a looping sound previously started with AudioPlayInstantLoop.
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioStopInstant(string soundName);

        // Apply a low-pass filter to a channel group (groupName: "SFX" or "BGM").
        // cutoff: frequency in Hz (10 - 22000). Pass 0 to remove the filter.
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetGroupLowPass(string groupName, float cutoff);

        // Set a loaded sound by name
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetSound(uint ID, string soundName);
    }
}
