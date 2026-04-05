using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Render Component  (implemented in eno_bridge_render.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetRenderWidth(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetRenderHeight(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetRenderSize(uint sceneObjectID, float w, float h);

        // textureName is the filename stem of the texture, e.g. "UI-MainMenu-Selected"
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetTexturePath(uint sceneObjectID, string textureName);

        // Like SetTexturePath but also resizes the render component to match the texture's native dimensions
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetTexturePathAndResize(uint sceneObjectID, string textureName);
        
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetUVStartX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetUVStartY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetUVEndX(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetUVEndY(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetUV(uint sceneObjectID, float startX, float startY, float endX, float endY);

        // Sets the RGBA color multiplier for the render component (r/g/b/a each 0.0–1.0).
        // Use to tint or fade a sprite: white (1,1,1,1) = no change; alpha 0 = fully transparent.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetColor(uint sceneObjectID, float r, float g, float b, float a);

        // -----------------------------------------------------------------------
        // Text Component  (implemented in eno_bridge_render.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static extern string GetText(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern void SetText(uint sceneObjectID, string text);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetFontSize(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetFontSize(uint sceneObjectID, float size);
    }
}
