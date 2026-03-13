namespace Eno
{
    // Preset: ACTOR — TransformComponent + Render + Collision + Physics + StateMachine
    //                 + Health + Damage + Visibility + Layer
    public class Actor : RigidBody2D
    {
        public Actor(uint id, NodeType type) : base(id, type) {}

        public float health
        {
            get => Bridge.GetHealth(ID);
            set => Bridge.SetHealth(ID, value);
        }

        public float maxHealth
        {
            get => Bridge.GetMaxHealth(ID);
            set => Bridge.SetMaxHealth(ID, value);
        }

        public bool isDead => Bridge.IsDead(ID);

        public void heal(float amount) => Bridge.HealEntity(ID, amount);

        public float damage
        {
            get => Bridge.GetDamage(ID);
            set => Bridge.SetDamageAmount(ID, value);
        }
    }
}
