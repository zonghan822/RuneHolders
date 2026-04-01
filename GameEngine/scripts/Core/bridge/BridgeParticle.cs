using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Particle System  (implemented in eno_bridge_particle.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void ParticleEmitBurst(uint sceneObjectID, int count);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void ParticleStartEmission(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void ParticleStopEmission(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleEmissionRate(uint sceneObjectID, float rate);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleMaxParticles(uint sceneObjectID, int max);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleLifetime(uint sceneObjectID, float minLife, float maxLife);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleSize(uint sceneObjectID, float minSize, float maxSize);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleVelocity(uint sceneObjectID, float minX, float minY, float maxX, float maxY);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleColorStart(uint sceneObjectID, float r, float g, float b, float a);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleColorEnd(uint sceneObjectID, float r, float g, float b, float a);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleGravity(uint sceneObjectID, float gx, float gy);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetParticleDrag(uint sceneObjectID, float drag);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetParticleIsEmitting(uint sceneObjectID);
    }
}
