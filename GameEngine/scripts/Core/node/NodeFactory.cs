namespace Eno
{
    public static class NodeFactory
    {
        private static readonly Dictionary<Type, NodeType> typeMap = new()
        {
            { typeof(Node2D),               NodeType.DEFAULT },
            { typeof(Sprite2D),             NodeType.SPRITE },
            { typeof(KinematicBody2D),      NodeType.KINEMATIC },
            { typeof(RigidBody2D),          NodeType.RIGIDBODY },
            { typeof(Actor),                NodeType.ACTOR },
            { typeof(Enemy),                NodeType.ENEMY },
            { typeof(Box2D),                NodeType.PUSHABLE_BOX },
            { typeof(Button),               NodeType.BUTTON },
            { typeof(AudioNode),            NodeType.AUDIO },
            { typeof(Camera),               NodeType.CAMERA },
            { typeof(Label),                NodeType.TEXT },
            { typeof(VideoNode),            NodeType.VIDEO },
            { typeof(ParticleEmitter),      NodeType.PARTICLE },
            { typeof(Canvas),               NodeType.CANVAS }
        };

        private static readonly Dictionary<NodeType, Func<uint, Node>> ctorMap = new()
        {
            { NodeType.DEFAULT, id          => new Node2D(id, NodeType.DEFAULT) },
            { NodeType.SPRITE, id           => new Sprite2D(id, NodeType.SPRITE) },
            { NodeType.KINEMATIC, id        => new KinematicBody2D(id, NodeType.KINEMATIC) },
            { NodeType.RIGIDBODY, id        => new RigidBody2D(id, NodeType.RIGIDBODY) },
            { NodeType.ACTOR, id            => new Actor(id, NodeType.ACTOR) },
            { NodeType.ENEMY, id            => new Enemy(id, NodeType.ENEMY) },
            { NodeType.PUSHABLE_BOX, id     => new Box2D(id, NodeType.PUSHABLE_BOX) },
            { NodeType.BUTTON, id           => new Button(id, NodeType.BUTTON) },
            { NodeType.AUDIO, id            => new AudioNode(id, NodeType.AUDIO) },
            { NodeType.CAMERA, id           => new Camera(id, NodeType.CAMERA) },
            { NodeType.TEXT, id             => new Label(id, NodeType.TEXT) },
            { NodeType.VIDEO, id            => new VideoNode(id, NodeType.VIDEO) },
            { NodeType.PARTICLE, id         => new ParticleEmitter(id, NodeType.PARTICLE) },
            { NodeType.CANVAS, id           => new Canvas(id, NodeType.CANVAS) }
        };

        public static Node? Create(uint id)
        {
            if (id == 0 || id == uint.MaxValue)
                return null;

            NodeType type = (NodeType)Bridge.GetNodeType(id);

            if (ctorMap.TryGetValue(type, out var ctor))
                return ctor(id);

            return new Node(id, type);
        }

        public static T Create<T>() where T : Node
        {
            NodeType type = typeMap[typeof(T)];

            uint id = Bridge.CreateNode((uint)type);

            if (ctorMap.TryGetValue(type, out var ctor))
                return (T)ctor(id);

            return (T)new Node(id, type);
        }
    }
}
