using System.Collections.Generic;

namespace Eno
{
    public static class EnemyColorRegistry
    {
        public static readonly Dictionary<uint, int> Colors = new();

        private static readonly Dictionary<int, int> _banCounts = new();

        public static void Register(uint id, int color) => Colors[id] = color;
        public static void Unregister(uint id) => Colors.Remove(id);

        public static void BanRune(int runeIndex)
        {
            _banCounts.TryGetValue(runeIndex, out int c);
            _banCounts[runeIndex] = c + 1;
        }

        public static void UnbanRune(int runeIndex)
        {
            if (!_banCounts.TryGetValue(runeIndex, out int c)) return;
            if (c <= 1) _banCounts.Remove(runeIndex);
            else _banCounts[runeIndex] = c - 1;
        }

        public static IEnumerable<int> ActiveBans => _banCounts.Keys;

        public static void Reset()
        {
            Colors.Clear();
            _banCounts.Clear();
        }
    }
}
