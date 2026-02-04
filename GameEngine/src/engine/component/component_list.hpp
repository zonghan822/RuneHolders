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
	X(NameComponent) \
	X(VisibilityComponent) \
	X(TransformComponent) \
	X(RenderComponent) \
	X(LayerComponent) \
    X(PlayerComponent) \
	X(CollisionComponent) \
	X(PhysicsComponent) \
	X(BoxComponent) \
	X(AIComponent) \
	X(ButtonComponent) \
	X(AudioComponent) \
	X(PrefabInstanceComponent) \
	X(AnimationControllerComponent)\
	X(CameraComponent) \
	X(RuneComponent) \
	X(RuneSystemStateComponent) \
	X(FireballComponent) \
	X(HealthComponent) \
	X(DamageComponent) \
	X(VulnerabilityComponent) \
	X(RuptureMarkComponent) \
	X(FloatingTextComponent) \

#define PRESET(Name, ...) using Name = std::tuple<__VA_ARGS__>;

#define PRESET_LIST \
	PRESET(DEFAULT, NameComponent, VisibilityComponent, LayerComponent) \
	PRESET(AUDIO, AudioComponent, LayerComponent) \
	PRESET(PUSHABLE_BOX, TransformComponent, PhysicsComponent, CollisionComponent, BoxComponent, RenderComponent, NameComponent) \
	PRESET(PLAYER, TransformComponent, NameComponent, PlayerComponent, CollisionComponent, PhysicsComponent, RenderComponent) \
	PRESET(EXIT_BUTTON, TransformComponent, ButtonComponent, DrawInfoComponent, VisibilityComponent, CollisionComponent) \


// Expanding PRESET_LIST within the component namespace
// to make use of the PRESET using macro
//
// This will allow the components to be visible to the list expansion
namespace ECSComponents { PRESET_LIST }
