namespace Eno
{
    // Preset: CAMERA — TransformComponent + CameraComponent
    public class Camera : Node
    {
        public Camera(uint id, NodeType type) : base(id, type) {}

        public Vector2 position
        {
            get => new Vector2(Bridge.GetPositionX(ID), Bridge.GetPositionY(ID));
            set => Bridge.SetPosition(ID, value.x, value.y);
        }
    }
}
