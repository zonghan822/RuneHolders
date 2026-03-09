namespace Eno
{
    // Preset: VIDEO — TransformComponent + RenderComponent + VideoComponent + Visibility + Layer
    public class VideoNode : Sprite2D
    {
        public VideoNode(uint id, NodeType type) : base(id, type) {}

        public void play()  => Bridge.VideoPlay(ID);
        public void pause() => Bridge.VideoPause(ID);
        public void stop()  => Bridge.VideoStop(ID);

        public bool isPlaying => Bridge.IsVideoPlaying(ID);

        public float time     => Bridge.GetVideoTime(ID);
        public float duration => Bridge.GetVideoDuration(ID);

        public bool loop
        {
            get => Bridge.GetVideoLoop(ID);
            set => Bridge.SetVideoLoop(ID, value);
        }
    }
}
