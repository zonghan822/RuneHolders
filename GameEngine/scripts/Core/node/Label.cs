namespace Eno
{
    // Preset: TEXT — TransformComponent + TextComponent + Visibility + Layer
    public class Label : Node2D
    {
        public Label(uint id, NodeType type) : base(id, type) {}

        public Vector2 position
        {
            get => new Vector2(Bridge.GetPositionX(ID), Bridge.GetPositionY(ID));
            set => Bridge.SetPosition(ID, value.x, value.y);
        }

        public string text
        {
            get => Bridge.GetText(ID);
            set => Bridge.SetText(ID, value);
        }

        public float fontSize
        {
            get => Bridge.GetFontSize(ID);
            set => Bridge.SetFontSize(ID, value);
        }
    }
}
