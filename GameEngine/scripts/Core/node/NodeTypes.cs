namespace Eno
{
    public enum NodeType : uint
    {
        EMPTY       = 0,    // No preset — default components from scene tree creation
        CUSTOM      = 1,    // Manually configured, no preset
        DEFAULT     = 2,    // VisibilityComponent + LayerComponent
        SPRITE      = 3,    // TransformComponent + RenderComponent + Visibility + Layer
        KINEMATIC   = 4,    // TransformComponent + Render + Collision + Visibility + Layer
        RIGIDBODY   = 5,    // TransformComponent + Render + Collision + Physics + Visibility + Layer
        ACTOR       = 6,    // TransformComponent + Render + Collision + Physics + StateMachine + Health + Damage + Visibility + Layer
        PUSHABLE_BOX = 7,   // TransformComponent + Render + Collision + Physics + Box + Visibility + Layer
        BUTTON      = 8,    // TransformComponent + Button + DrawInfo + Visibility + Collision
        AUDIO       = 9,    // AudioComponent + Layer
        CAMERA      = 10,   // TransformComponent + CameraComponent
        TEXT        = 11,   // TransformComponent + TextComponent + Visibility + Layer
        PARTICLE    = 12,   // TransformComponent + ParticleEmitter + Visibility + Layer
        VIDEO       = 13,   // TransformComponent + RenderComponent + VideoComponent + Visibility + Layer
        ENEMY       = 14,   // ACTOR components + AIComponent
    }
}
