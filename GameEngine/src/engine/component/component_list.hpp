/*============================================================================================
* @file         component_id.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        XMacro definition for providing a ssot for component registering
* 
* All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#pragma once


#define COMPONENT_LIST \
	X(VisibilityComponent) \
	X(TransformComponent) \
	X(RenderComponent) \
	X(LayerComponent) \
	X(CollisionComponent) \
	X(PhysicsComponent) \
	X(BoxComponent) \
	X(AIComponent) \
	X(ButtonComponent) \
	X(AudioComponent) \
	X(AnimationControllerComponent)\
	X(CameraComponent) \
	X(HealthComponent) \
	X(DamageComponent) \
	X(StateMachineComponent) \
	X(VulnerabilityComponent) \
	X(ParticleEmitterComponent) \
	X(TextComponent) \
	X(VideoComponent) \

#define PRESET(Name, ...) using Name = std::tuple<__VA_ARGS__>;

#define PRESET_LIST \
	/* Blank node — layer ordering + visibility only */ \
	PRESET(DEFAULT,      VisibilityComponent, LayerComponent) \
	/* Static visual — no physics or collision */ \
	PRESET(SPRITE,       TransformComponent, RenderComponent, VisibilityComponent, LayerComponent) \
	/* Visual with collision but no physics (walls, platforms, triggers) */ \
	PRESET(KINEMATIC,    TransformComponent, RenderComponent, CollisionComponent, VisibilityComponent, LayerComponent) \
	/* Full physics object */ \
	PRESET(RIGIDBODY,    TransformComponent, RenderComponent, CollisionComponent, PhysicsComponent, VisibilityComponent, LayerComponent) \
	/* Full character setup (player / enemy) */ \
	PRESET(ACTOR,        TransformComponent, RenderComponent, CollisionComponent, PhysicsComponent, StateMachineComponent, HealthComponent, DamageComponent, VulnerabilityComponent, VisibilityComponent, LayerComponent) \
	/* Object that can be pushed around */ \
	PRESET(PUSHABLE_BOX, TransformComponent, RenderComponent, CollisionComponent, PhysicsComponent, BoxComponent, VisibilityComponent, LayerComponent) \
	/* UI button */ \
	PRESET(BUTTON,       TransformComponent, ButtonComponent, DrawInfoComponent, VisibilityComponent, CollisionComponent) \
	/* Audio source */ \
	PRESET(AUDIO,        AudioComponent, LayerComponent) \
	/* Camera */ \
	PRESET(CAMERA,       TransformComponent, CameraComponent) \
	/* World or UI text */ \
	PRESET(TEXT,         TransformComponent, TextComponent, VisibilityComponent, LayerComponent) \
	/* Particle emitter */ \
	PRESET(PARTICLE,     TransformComponent, ParticleEmitterComponent, VisibilityComponent, LayerComponent) \
	/* Video player */ \
	PRESET(VIDEO,        TransformComponent, RenderComponent, VideoComponent, VisibilityComponent, LayerComponent) \


// Expanding PRESET_LIST within the component namespace
// to make use of the PRESET using macro
//
// This will allow the components to be visible to the list expansion
namespace ECSComponents { PRESET_LIST }
