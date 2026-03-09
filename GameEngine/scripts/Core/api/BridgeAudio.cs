using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Audio Component  (implemented in eno_bridge_audio.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioPlay(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AudioStop(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetAudioVolume(uint sceneObjectID);

        // volume: 0 – 100
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioVolume(uint sceneObjectID, float volume);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetAudioPitch(uint sceneObjectID);

        // pitch: 1.0 = normal
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAudioPitch(uint sceneObjectID, float pitch);
    }
}
