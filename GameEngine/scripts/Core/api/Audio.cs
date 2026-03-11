namespace Eno
{
    // Global audio control — master volume, channel groups, and persistence.
    // Per-entity playback is handled by the AudioNode class.
    public static class Audio
    {
        // ── Master volume ─────────────────────────────────────────────────────

        // Master volume in the range 0–100.
        public static float masterVolume
        {
            get => Bridge.GetMasterVolume();
            set => Bridge.SetMasterVolume(value);
        }

        // ── Channel groups ────────────────────────────────────────────────────
        // groupName is typically "BGM" or "SFX". Volume range: 0–100.

        public static float getGroupVolume(string groupName)
            => Bridge.GetGroupVolume(groupName);

        public static void setGroupVolume(string groupName, float volume)
            => Bridge.SetGroupVolume(groupName, volume);

        // ── Persistence ───────────────────────────────────────────────────────

        // Write all three volumes to config.txt so they survive scene changes and restarts.
        public static void saveVolumes(float master, float bgm, float sfx)
            => Bridge.SaveAudioVolumes(master, bgm, sfx);

        // Read a single volume from config.txt by key ("Audio Master", "Audio BGM", "Audio SFX").
        // Returns defaultVal when the key is absent.
        public static float loadVolume(string key, float defaultVal = 100f)
            => Bridge.LoadAudioVolume(key, defaultVal);
    }
}
