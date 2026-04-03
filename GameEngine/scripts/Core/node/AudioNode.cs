namespace Eno
{
    // Preset: AUDIO — AudioComponent + LayerComponent
    public class AudioNode : Node
    {
        public AudioNode(uint id, NodeType type) : base(id, type) {}

        public void play()  => Bridge.AudioPlay(ID);
        public void stop()  => Bridge.AudioStop(ID);
        public void spatialPlay(uint id, float x, float y) => Bridge.AudioPlayInstant(id, x, y);

        public bool isPlaying => Bridge.IsAudioPlaying(ID) != 0;

        public float volume
        {
            get => Bridge.GetAudioVolume(ID);
            set => Bridge.SetAudioVolume(ID, value);
        }

        public float pitch
        {
            get => Bridge.GetAudioPitch(ID);
            set => Bridge.SetAudioPitch(ID, value);
        }

        public string sound
        {
            set => Bridge.SetSound(ID, value);
        }

        // Enable spatial audio: volume falls off with distance, pan follows X offset from camera
        public bool spatialAudio
        {
            set => Bridge.SetAudioSpatial(ID, value ? 1 : 0);
        }

        // World-unit distance at which volume reaches zero (default 500)
        public float maxAudibleDistance
        {
            set => Bridge.SetAudioMaxDistance(ID, value);
        }

        // Route this node through a different channel group ("BGM" or "SFX")
        public string group
        {
            set => Bridge.SetAudioGroup(ID, value);
        }
    }
}
