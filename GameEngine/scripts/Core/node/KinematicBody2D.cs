namespace Eno
{
    // Preset: KINEMATIC — TransformComponent + Render + Collision + Visibility + Layer
    public class KinematicBody2D : Sprite2D
    {
        public KinematicBody2D(uint id, NodeType type) : base(id, type) {}

        public bool isOverlappingPoint(float x, float y) => Bridge.IsOverlappingPoint(ID, x, y);
        public bool isColliding(Node other)              => Bridge.IsColliding(ID, other.ID);
        public bool colliderActive
        {
            get => Bridge.GetColliderActive(ID);
            set => Bridge.SetColliderActive(ID, value);
        }

    }
}
