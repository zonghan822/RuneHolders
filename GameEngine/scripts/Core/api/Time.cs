namespace Eno
{
    // Static wrapper for time-related bridge functions.
    public static class Time
    {
        // Seconds elapsed since the last frame.
        public static float deltaTime => Bridge.GetDeltaTime();
    }
}
