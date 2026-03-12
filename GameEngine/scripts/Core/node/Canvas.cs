namespace Eno
{
    // Preset: CANVAS — TransformComponent + RenderComponent + CanvasComponent + Visibility + Layer
    public class Canvas : Node2D
    {
        public Canvas(uint id, NodeType type) : base(id, type) {}

        // Whether this canvas is active (shows/hides all child UI elements)
        public bool active
        {
            get => Bridge.GetCanvasActive(ID);
            set => Bridge.SetCanvasActive(ID, value);
        }
    }
}
