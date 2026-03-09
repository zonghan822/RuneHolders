namespace Eno
{
    // Preset: AUDIO — AudioComponent + LayerComponent
    public class AudioNode : Node
    {
        public AudioNode(uint id, NodeType type) : base(id, type) {}

        public void play()  => Bridge.AudioPlay(ID);
        public void stop()  => Bridge.AudioStop(ID);

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
    }
}
