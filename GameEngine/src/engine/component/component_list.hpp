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

#define PRESET(Name, ...) using Name = std::tuple<__VA_ARGS__>;

#define PRESET_LIST \
	PRESET(DEFAULT, VisibilityComponent, LayerComponent) \
	PRESET(AUDIO, AudioComponent, LayerComponent) \
	PRESET(PUSHABLE_BOX, TransformComponent, PhysicsComponent, CollisionComponent, BoxComponent, RenderComponent) \
	PRESET(PLAYER, TransformComponent, CollisionComponent, PhysicsComponent, RenderComponent) \
	PRESET(EXIT_BUTTON, TransformComponent, ButtonComponent, DrawInfoComponent, VisibilityComponent, CollisionComponent) \


// Expanding PRESET_LIST within the component namespace
// to make use of the PRESET using macro
//
// This will allow the components to be visible to the list expansion
namespace ECSComponents { PRESET_LIST }
