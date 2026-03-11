namespace Eno
{
    // Global window and application control.
    public static class Window
    {
        // ── Size ──────────────────────────────────────────────────────────────

        public static int width  => Bridge.GetWindowWidth();
        public static int height => Bridge.GetWindowHeight();

        // ── Resolution ────────────────────────────────────────────────────────

        public static void setResolution(int width, int height)
            => Bridge.SetResolution(width, height);

        public static void increaseResolution() => Bridge.IncreaseResolution();
        public static void decreaseResolution() => Bridge.DecreaseResolution();

        // ── Display mode ──────────────────────────────────────────────────────

        public static void setVSync(bool enabled) => Bridge.SetVSync(enabled);
        public static void toggleFullscreen()     => Bridge.ToggleFullscreen();

        // ── Performance ───────────────────────────────────────────────────────

        public static float fps => Bridge.GetFPS();

        // ── Pause ─────────────────────────────────────────────────────────────

        public static bool paused
        {
            get => Bridge.GetIsPaused();
            set => Bridge.SetPaused(value);
        }

        // ── Lifecycle ─────────────────────────────────────────────────────────

        public static bool shouldClose => Bridge.GetShouldClose();
        public static void requestClose()  => Bridge.RequestClose();
    }
}
