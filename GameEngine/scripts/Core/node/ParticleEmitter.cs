namespace Eno
{
    // Preset: PARTICLE — TransformComponent + RenderComponent + ParticleEmitterComponent + Visibility + Layer
    public class ParticleEmitter : Sprite2D
    {
        public ParticleEmitter(uint id, NodeType type) : base(id, type) {}

        public void emitBurst(int count) => Bridge.ParticleEmitBurst(ID, count);
        public void startEmission() => Bridge.ParticleStartEmission(ID);
        public void stopEmission() => Bridge.ParticleStopEmission(ID);

        public float emissionRate
        {
            set => Bridge.SetParticleEmissionRate(ID, value);
        }

        public int maxParticles
        {
            get => Bridge.GetParticleMaxParticles(ID);
            set => Bridge.SetParticleMaxParticles(ID, value);
        }

        public bool isEmitting
        {
            get => Bridge.GetParticleIsEmitting(ID);
        }

        public void setLifetime(float min, float max) => Bridge.SetParticleLifetime(ID, min, max);
        public void setSize(float min, float max) => Bridge.SetParticleSize(ID, min, max);
        public void setVelocity(float minX, float minY, float maxX, float maxY) => Bridge.SetParticleVelocity(ID, minX, minY, maxX, maxY);
        public void setColorStart(float r, float g, float b, float a) => Bridge.SetParticleColorStart(ID, r, g, b, a);
        public void setColorEnd(float r, float g, float b, float a) => Bridge.SetParticleColorEnd(ID, r, g, b, a);
        public void setGravity(float gx, float gy) => Bridge.SetParticleGravity(ID, gx, gy);

        public float drag
        {
            set => Bridge.SetParticleDrag(ID, value);
        }
    }
}
