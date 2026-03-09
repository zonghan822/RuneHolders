 namespace Eno
{
    public class Box2D : Sprite2D
    {
        public Box2D(uint id, NodeType type) : base(id, type) {}

        public Vector2 velocity
        {
            get => new Vector2(Bridge.GetVelocityX(ID), Bridge.GetVelocityY(ID));
            set => Bridge.SetVelocity(ID, value.x, value.y);
        }

        public void addForce(float x, float y) => Bridge.AddForce(ID, x, y);
        public void addForce(Vector2 f)         => Bridge.AddForce(ID, f.x, f.y);

        public float mass
        {
            get => Bridge.GetMass(ID);
            set => Bridge.SetMass(ID, value);
        }

        public bool useGravity
        {
            get => Bridge.GetUseGravity(ID);
            set => Bridge.SetUseGravity(ID, value);
        }

        public bool isKinematic
        {
            get => Bridge.GetIsKinematic(ID);
            set => Bridge.SetIsKinematic(ID, value);
        }
    }
}
 