namespace Eno
{
    public struct Vector2
    {
        public float x;
        public float y;

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public static readonly Vector2 Zero = new(0f, 0f);
        public static readonly Vector2 One  = new(1f, 1f);
        public static readonly Vector2 Up   = new(0f, -1f);
        public static readonly Vector2 Down = new(0f,  1f);
        public static readonly Vector2 Left = new(-1f, 0f);
        public static readonly Vector2 Right = new(1f, 0f);

        public static Vector2 operator +(Vector2 a, Vector2 b) => new(a.x + b.x, a.y + b.y);
        public static Vector2 operator -(Vector2 a, Vector2 b) => new(a.x - b.x, a.y - b.y);
        public static Vector2 operator *(Vector2 a, float s)   => new(a.x * s,   a.y * s);
        public static Vector2 operator *(float s, Vector2 a)   => new(a.x * s,   a.y * s);

        public override string ToString() => $"({x}, {y})";
    }
}
