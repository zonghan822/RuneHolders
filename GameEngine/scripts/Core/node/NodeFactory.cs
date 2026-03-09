namespace Eno
{
    public static class NodeFactory
    {
        // Create a typed node wrapper from an existing scene object ID
        public static Node Create(uint id)
        {
            if (id == 0 || id == uint.MaxValue)
                return null;

            NodeType type = (NodeType)Bridge.GetNodeType(id);

            return type switch
            {
                NodeType.DEFAULT     => new Node2D(id, type),
                NodeType.SPRITE      => new Sprite2D(id, type),
                NodeType.KINEMATIC   => new KinematicBody2D(id, type),
                NodeType.RIGIDBODY   => new RigidBody2D(id, type),
                NodeType.ACTOR       => new Actor(id, type),
                NodeType.PUSHABLE_BOX => new RigidBody2D(id, type),
                NodeType.BUTTON      => new Node2D(id, type),
                NodeType.AUDIO       => new AudioNode(id, type),
                NodeType.CAMERA      => new Camera(id, type),
                NodeType.TEXT        => new Label(id, type),
                NodeType.PARTICLE    => new ParticleEmitter(id, type),
                _                    => new Node(id, type),
            };
        }

        // Spawn a new node from a preset name, optionally as a child of parentID
        public static Node Spawn(string preset, string name, uint parentID = uint.MaxValue)
        {
            uint id = Bridge.SpawnNode(preset, name, parentID);
            return Create(id);
        }
    }
}
