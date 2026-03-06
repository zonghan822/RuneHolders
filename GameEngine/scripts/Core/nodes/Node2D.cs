namespace Eno
{
    public class Node2D : Node
    {
        internal Node2D(uint id) : base(id) {}

        // Visibility
        public uint z_index
        {
            get => Bridge.GetZIndex(id);
            set => Bridge.SetZIndex(id, value);
        }

        public bool visible
        {
            get => Bridge.GetVisibility(id);
            set => Bridge.SetVisibility(id, value);
        }
    }
}