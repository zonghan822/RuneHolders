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
        private Node _self = null!;
        public Node self
        {
            get
            {
                if (_self != null)
                    return _self;

                Node? nodeBase = NodeFactory.Create(ID);

                if (nodeBase == null)
                    throw new InvalidOperationException($"Node '{ID}' could not be created.");

                _self = nodeBase;
                return _self;
            }
        }

        // Self referencing method
        public T? Self<T>() where T : Node
        {
            return self as T;
        }

        // Create a new node of T type
        public T? CreateNode<T>() where T : Node
        {
            return NodeFactory.Create<T>();
        }

        // Free a node from the scene tree
        public void FreeNode(Node? node)
        {
            if (node == null)
                throw new ArgumentNullException(nameof(node));
            
            Bridge.FreeNode(node.ID);
        }

        // Get another node within the scene tree
        public T? GetNode<T>(string name) where T : Node
        {
            Node? node = NodeFactory.Create(Bridge.GetNode(name));
            if (node == null)
                throw new InvalidOperationException($"Node '{name}' not found.");

            if (node is not T tNode)
                throw new InvalidCastException($"Node '{name}' is not of type {typeof(T).Name}.");

            return tNode;
        }

        // Get a child of the current node
        public T? GetChild<T>(uint index) where T : Node
        {
            Node? node = NodeFactory.Create(Bridge.GetChild(ID, index));
            if (node == null || node.ID == 0)
                return null;

            if (node is not T tNode)
                throw new InvalidCastException($"Node '{ID}''s Child is not of type {typeof(T).Name}.");

            return tNode;
        }

        // Add a child
        public void AddChild(Node? child)
        {
            if (child == null)
                throw new ArgumentNullException(nameof(child));
            
            Bridge.AddChild(child.ID, ID);
        }

        // Remove a child
        public void Unparent(Node? node)
        {
            if (node == null)
                throw new ArgumentNullException(nameof(node));
            
            Bridge.Unparent(node.ID);
        }

        // Get parent node of the current node
        public T? GetParent<T>() where T : Node
        {
            Node? node = NodeFactory.Create(Bridge.GetParentNode(ID));
            if (node == null || node.ID == 0)
                return null;

            if (node is not T tNode)
                throw new InvalidCastException($"Node '{ID}''s Parent is not of type {typeof(T).Name}.");

            return tNode;
        }

        // Get number of children nodes
        public uint GetChildCount()
        {
            return Bridge.GetChildCount(ID);
        }
    }
}
