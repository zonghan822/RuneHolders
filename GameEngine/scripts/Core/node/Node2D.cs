namespace Eno
{
    public class Node2D : Node
    {
        public Node2D(uint id, NodeType type) : base(id, type) {}

        // Visibility
        public uint z_index
        {
            get => Bridge.GetZIndex(ID);
            set => Bridge.SetZIndex(ID, value);
        }

        public bool visible
        {
            get => Bridge.GetVisibility(ID);
            set => Bridge.SetVisibility(ID, value);
        }
    }
}