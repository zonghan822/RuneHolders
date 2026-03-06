namespace Eno
{
    // Nodes with name, visibility and layer components
    public class Node
    {
        public uint ID { get; }
        public NodeType Type { get; }

        public Node(uint ID, NodeType type)
        {
            this.ID = ID;
            this.Type = type;
        }

        public Node GetChild(uint index)
        {
            uint childID = Bridge.GetChild(ID, index);
            if (childID == 0) return null;
            return NodeFactory.Create(childID);
        }
    }
}