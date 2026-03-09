using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // State Machine  (implemented in eno_bridge_state.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void ChangeState(uint sceneObjectID, string stateName);
    }
}
