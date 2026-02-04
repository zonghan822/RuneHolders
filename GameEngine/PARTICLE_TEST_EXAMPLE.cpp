#include "game_logic.hpp"
#include <GLFW/glfw3.h>
#include "particle/particle_presets.hpp"
#include "particle/particle_component.hpp"
#include <typeindex>

void GameApplication::testParticleSystem() {
	auto& entityManager = engine_.getEntityManager();
	auto& input = engine_.getInputManager();

	if (input.isKeyPressed(GLFW_KEY_P)) {
		auto mousePos = input.getMousePosition();
		float worldX = static_cast<float>(mousePos.first) + editorCameraX_;
		float worldY = static_cast<float>(mousePos.second) + editorCameraY_;

		auto particleEntity = entityManager.create<
			ECSComponents::TransformComponent,
			ECSComponents::ParticleEmitterComponent
		>(
			ECSComponents::TransformComponent{{worldX, worldY}, {0,0}, {1,1}},
			Particles::Presets::CreateFireball()
		);

		testParticleEntityID_ = particleEntity.id();
		particleTestSpawned_ = true;
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_O)) {
		auto* particleSystem = engine_.getParticleSystem();
		if (particleSystem) {
			auto it = entityManager.getMap().find(testParticleEntityID_);
			if (it != entityManager.getMap().end()) {
				auto& entityLoc = it->second;

				bool hasEmitter = false;
				for (const auto& type : entityLoc.archetype->types_) {
					if (type == std::type_index(typeid(ECSComponents::ParticleEmitterComponent))) {
						hasEmitter = true;
						break;
					}
				}

				if (hasEmitter) {
					size_t index = entityLoc.archetype->findComponentIndex<ECSComponents::ParticleEmitterComponent>();
					auto& emitter = entityLoc.archetype->chunks_[entityLoc.chunkIndex]
						->retrieve<ECSComponents::ParticleEmitterComponent>(index, entityLoc.entityIndexInChunk);

					emitter.isEmitting = !emitter.isEmitting;
				}
			}
		}
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_B)) {
		auto* particleSystem = engine_.getParticleSystem();
		if (particleSystem) {
			particleSystem->EmitBurst(entityManager, testParticleEntityID_, 50);
		}
	}

	if (particleTestSpawned_ && input.isKeyPressed(GLFW_KEY_K)) {
		auto it = entityManager.getMap().find(testParticleEntityID_);
		if (it != entityManager.getMap().end()) {
			ECS::Entity(testParticleEntityID_).destroy(entityManager);
			particleTestSpawned_ = false;
		}
	}
}

Available Presets:
Particles::Presets::CreateFireball()
Particles::Presets::CreateIceShard()
Particles::Presets::CreateLightning()
Particles::Presets::CreateMagicAura()
Particles::Presets::CreateExplosion()
Particles::Presets::CreateHealingSparkles()
Particles::Presets::CreateSmokeTrail()

Controls:
P = Spawn particle emitter at mouse
O = Toggle emission on/off
B = Emit burst of 50 particles
K = Delete particle emitter
