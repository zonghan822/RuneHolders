using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Video Component  (implemented in eno_bridge_video.cpp)
        // -----------------------------------------------------------------------

        /// <summary>Start (or resume) playback of the entity's video.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void VideoPlay(uint sceneObjectID);

        /// <summary>Pause playback without rewinding.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void VideoPause(uint sceneObjectID);

        /// <summary>Stop playback and rewind to the beginning.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void VideoStop(uint sceneObjectID);

        /// <summary>Returns true while the video is actively playing.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsVideoPlaying(uint sceneObjectID);

        /// <summary>Current playback position in seconds.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetVideoTime(uint sceneObjectID);

        /// <summary>Total duration of the video in seconds.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetVideoDuration(uint sceneObjectID);

        /// <summary>Enable or disable looping for this entity's video.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetVideoLoop(uint sceneObjectID, bool loop);

        /// <summary>Returns true if looping is enabled.</summary>
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetVideoLoop(uint sceneObjectID);
    }
}
