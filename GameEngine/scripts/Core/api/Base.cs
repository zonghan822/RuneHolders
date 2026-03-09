using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Eno
{
    public class ScriptBase
    {
        public static InputManager InputMng => InputManager.Instance;

        public static void Synchronize()
        {
            InputMng.Update();
        }

        // Script ID
        public uint ID { get; set; }

        // Get self
        private Node _self;
        public Node self
        {
            get
            {
                if (_self != null)
                    return _self;

                Node nodeBase = NodeFactory.Create(ID);

                switch(nodeBase.Type)
                {
                    case NodeType.DEFAULT:
                        _self = new Node2D(nodeBase.ID, nodeBase.Type);
                        break;
                    default:
                        _self = nodeBase;
                        break;
                }

                return _self;
            }
        }

        public T Self<T>() where T : Node
        {
            return self as T;
        }

        // Get another node within the scene tree
        public T GetNode<T>(string name) where T : Node
        {
            Node node = NodeFactory.Create(Bridge.GetNode(name));
            if (node == null)
                throw new InvalidOperationException($"Node '{name}' not found.");

            if (node is not T tNode)
                throw new InvalidCastException($"Node '{name}' is not of type {typeof(T).Name}.");

            return tNode;
        }

        // Get a child of the current node
        public T GetChild<T>(uint index) where T : Node
        {
            Node node = NodeFactory.Create(Bridge.GetChild(ID, index));
            if (node == null)
                throw new InvalidOperationException($"Node '{ID}''s child not found.");

            if (node is not T tNode)
                throw new InvalidCastException($"Node '{ID}''s child is not of type {typeof(T).Name}.");

            return tNode;
        }
    }
}
