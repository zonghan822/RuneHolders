namespace Eno
{
    // Preset: BUTTON — TransformComponent + ButtonComponent + DrawInfo + Visibility + Collision
    public class Button : Node2D
    {
        public Button(uint id, NodeType type) : base(id, type) {}

        public Vector2 position
        {
            get => new Vector2(Bridge.GetPositionX(ID), Bridge.GetPositionY(ID));
            set => Bridge.SetPosition(ID, value.x, value.y);
        }

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
    }
}
