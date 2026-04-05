namespace Eno
{
    // Nodes with name, visibility and layer components
    public class Node : ScriptBase
    {
        public new uint ID { get; }
        public NodeType Type { get; }

        public Node(uint ID, NodeType type)
        {
            this.ID = ID;
            this.Type = type;
        }
    }
}