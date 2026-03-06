namespace Eno
{
    public static class NodeFactory
    {
        public static Node Create(uint id)
        {
            if (id == 0)
                return null;
            
            NodeType type = (NodeType)Bridge.GetNodeType(id);

            return type switch
            {
                NodeType.DEFAULT => new Node2D(id, type),
                _ => new Node(id, type),
            };
        }
    }
}