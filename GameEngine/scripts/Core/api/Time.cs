namespace Eno
{
    // Static wrapper for time-related bridge functions.
    public static class Time
    {
        // Seconds elapsed since the last frame (affected by timeScale).
        public static float deltaTime => Bridge.GetDeltaTime();

        // Real seconds elapsed since the last frame (never affected by timeScale).
        public static float unscaledDeltaTime => Bridge.GetUnscaledDeltaTime();

        // Global time scale: 1.0 = normal, <1.0 = slow motion.
        public static float timeScale
        {
            get => Bridge.GetTimeScale();
            set => Bridge.SetTimeScale(value);
        }
    }
}
