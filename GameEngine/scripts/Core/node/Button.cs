namespace Eno
{
    // Preset: BUTTON — TransformComponent + DrawInfoComponent + AnimationControllerComponent
    //                  + ButtonComponent + VisibilityComponent + CollisionComponent
    // DrawInfoComponent is an alias for RenderComponent, so all Sprite2D bridge calls work.
    public class Button : Sprite2D
    {
        public Button(uint id, NodeType type) : base(id, type) {}
    }
}
