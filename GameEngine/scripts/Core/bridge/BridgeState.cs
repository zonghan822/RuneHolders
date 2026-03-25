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

        // Returns true if state transitions are currently disabled for an entity.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetStateChangeLocked(uint sceneObjectID);

        // Enable (false) or disable (true) state transitions for an entity.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetStateChangeLocked(uint sceneObjectID, bool locked);

        // Returns the current state name of an entity's state machine.
        public static string GetCurrentState(uint sceneObjectID)
        {
            byte[] buf = new byte[32];
            GetCurrentState_Native(sceneObjectID, buf, buf.Length);
            int len = System.Array.IndexOf(buf, (byte)0);
            return System.Text.Encoding.ASCII.GetString(buf, 0, len < 0 ? buf.Length : len);
        }

        [DllImport("Engine_shared.dll", EntryPoint = "GetCurrentState", CallingConvention = CallingConvention.StdCall)]
        private static extern void GetCurrentState_Native(
            uint sceneObjectID,
            [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] outBuffer,
            int bufferSize);
    }
}
