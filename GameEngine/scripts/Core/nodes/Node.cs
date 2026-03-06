namespace Eno
{
    // Nodes with name, visibility and layer components
    public class Node
    {
        public uint id;

        public Node(uint id)
        {
            this.id = id;
        }

        public Node GetChild(uint index)
        {
            uint childID = Bridge.GetChild(id, index);
            return NodeFactory.Create(childID);
        }

        public uint ID => id;
    }
}