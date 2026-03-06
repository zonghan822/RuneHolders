namespace Eno
{
    internal static class NodeFactory
    {
        public static Node Create(uint id)
        {
            NodeType type = (NodeType)Bridge.GetNodeType(id);
            return type switch
            {
                NodeType.DEFAULT => new Node2D(id),
                _ => new Node(id),
            };
        }
    }
}