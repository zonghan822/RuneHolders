/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         component_list.hpp
* @author       ZongHan Law (zonghan.law@digipen.edu)   38%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   26%
* @co-author    Caven Koh (caven.koh@digipen.edu)   24%
* @date         10/3/2025
* @brief        XMacro definition for providing a ssot for component registering
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
	X(CanvasComponent) \
	X(TextComponent) \
	X(VideoComponent) \
//particle emitter omitted

#define PRESET(Name, ...) using Name = std::tuple<__VA_ARGS__>;

#define PRESET_LIST \
	/* Blank node — layer ordering + visibility only */ \
	PRESET(DEFAULT,      VisibilityComponent, LayerComponent) \
	/* Static visual — no physics or collision */ \
	PRESET(SPRITE,       TransformComponent, RenderComponent, AnimationControllerComponent, StateMachineComponent, VisibilityComponent, LayerComponent) \
	/* Visual with collision but no physics (walls, platforms, triggers) */ \
	PRESET(KINEMATIC,    TransformComponent, RenderComponent, AnimationControllerComponent, StateMachineComponent, CollisionComponent, VisibilityComponent, LayerComponent) \
	/* Full physics object */ \
	PRESET(RIGIDBODY,    TransformComponent, RenderComponent, AnimationControllerComponent, StateMachineComponent, CollisionComponent, PhysicsComponent, VisibilityComponent, LayerComponent) \
	/* Full character setup (player / enemy) */ \
	PRESET(ACTOR,        TransformComponent, RenderComponent, AnimationControllerComponent, StateMachineComponent, CollisionComponent, PhysicsComponent, HealthComponent, DamageComponent, VisibilityComponent, LayerComponent) \
	/* Object that can be pushed around */ \
	PRESET(PUSHABLE_BOX, TransformComponent, RenderComponent, AnimationControllerComponent, StateMachineComponent, CollisionComponent, PhysicsComponent, BoxComponent, VisibilityComponent, LayerComponent) \
	/* UI button */ \
	PRESET(BUTTON,       TransformComponent, ButtonComponent, DrawInfoComponent, AnimationControllerComponent, VisibilityComponent, CollisionComponent) \
	/* Audio source */ \
	PRESET(AUDIO,        AudioComponent, LayerComponent) \
	/* Camera */ \
	PRESET(CAMERA,       TransformComponent, CameraComponent) \
	/* World or UI text */ \
	PRESET(TEXT,         TransformComponent, TextComponent, VisibilityComponent, LayerComponent) \
	/* Video player */ \
	PRESET(VIDEO,        TransformComponent, RenderComponent, VideoComponent, VisibilityComponent, LayerComponent) \
	/* AI-driven enemy — ACTOR components + AIComponent for pathfinding and behaviour */ \
	PRESET(ENEMY,        TransformComponent, RenderComponent, AnimationControllerComponent, CollisionComponent, PhysicsComponent, StateMachineComponent, HealthComponent, DamageComponent, AIComponent, VisibilityComponent, LayerComponent) \
	/* Canvas — UI root that positions children relative to the viewport */ \
	PRESET(CANVAS,		 TransformComponent, RenderComponent, CanvasComponent, VisibilityComponent, LayerComponent) \
//particle emitter omitted

// Expanding PRESET_LIST within the component namespace
// to make use of the PRESET using macro
//
// This will allow the components to be visible to the list expansion
namespace ECSComponents { PRESET_LIST }
