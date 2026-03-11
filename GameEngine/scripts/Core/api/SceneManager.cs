using System.Text;

namespace Eno
{
    // Global scene management — change or query the active scene from any script.
    public static class SceneManager
    {
        // Queue a scene change to be executed at the end of the current frame.
        // Safe to call from any script's Update().
        // sceneName is the scene filename, e.g. "Level.txt".
        public static void changeScene(string sceneName)
            => Bridge.ChangeScene(sceneName);

        // Name of the currently loaded scene (e.g. "MainMenu.txt").
        public static string currentScene
        {
            get
            {
                byte[] buf = new byte[256];
                Bridge.GetCurrentScene(buf, buf.Length);
                int len = System.Array.IndexOf(buf, (byte)0);
                return Encoding.ASCII.GetString(buf, 0, len < 0 ? buf.Length : len);
            }
        }
    }
}
