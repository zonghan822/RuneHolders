namespace Eno
{
    // Preset: SPRITE — TransformComponent + RenderComponent + Visibility + Layer
    public class Sprite2D : Node2D
    {
        public Sprite2D(uint id, NodeType type) : base(id, type) {}

        public Vector2 position
        {
            get => new Vector2(Bridge.GetPositionX(ID), Bridge.GetPositionY(ID));
            set => Bridge.SetPosition(ID, value.x, value.y);
        }

        // Engine stores rotation as Vec2 (x = horizontal angle component, y = vertical).
        public Vector2 rotation
        {
            get => new Vector2(Bridge.GetRotationX(ID), Bridge.GetRotationY(ID));
            set => Bridge.SetRotation(ID, value.x, value.y);
        }

        public Vector2 scale
        {
            get => new Vector2(Bridge.GetScaleX(ID), Bridge.GetScaleY(ID));
            set => Bridge.SetScale(ID, value.x, value.y);
        }

        public float width
        {
            get => Bridge.GetRenderWidth(ID);
        }

        public float height
        {
            get => Bridge.GetRenderHeight(ID);
        }

        public void setSize(float w, float h) => Bridge.SetRenderSize(ID, w, h);

        public Vector2 uvStart
        {
            get => new Vector2(Bridge.GetUVStartX(ID), Bridge.GetUVStartY(ID));
            set => Bridge.SetUV(ID, value.x, value.y, Bridge.GetUVEndX(ID), Bridge.GetUVEndY(ID));
        }

        public Vector2 uvEnd
        {
            get => new Vector2(Bridge.GetUVEndX(ID), Bridge.GetUVEndY(ID));
            set => Bridge.SetUV(ID, Bridge.GetUVStartX(ID), Bridge.GetUVStartY(ID), value.x, value.y);
        }

        public void setUV(Vector2 start, Vector2 end) => Bridge.SetUV(ID, start.x, start.y, end.x, end.y);

        public string texture
        {
            set => Bridge.SetTexturePath(ID, value);
        }

        // Sets the texture and resizes the render component to the texture's native dimensions.
        public void setTextureAndResize(string path) => Bridge.SetTexturePathAndResize(ID, path);

        public string animation
        {
            get => Bridge.GetCurrentAnimation(ID);
            set => Bridge.SetAnimation(ID, value);
        }

        public string currentState => Bridge.GetCurrentState(ID);

        public bool stateChangeLocked
        {
            get => Bridge.GetStateChangeLocked(ID);
            set => Bridge.SetStateChangeLocked(ID, value);
        }

        public void changeState(string state) => Bridge.ChangeState(ID, state);

        public void setAnimationLooping(bool loop)             => Bridge.SetAnimationLooping(ID, loop);
        public void setAnimationFrameDuration(float frameDur)  => Bridge.SetAnimationFrameDuration(ID, frameDur);
        public uint getAnimationFrameCount()                   => Bridge.GetAnimationFrameCount(ID);
    }
}
