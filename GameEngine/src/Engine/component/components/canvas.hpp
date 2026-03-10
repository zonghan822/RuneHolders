/*============================================================================================
* @file         canvas.hpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         27/02/2026
* @brief        This file contains the declaration of the canvas component class for UI elements
*//*========================================================================================*/
#pragma once

// Signals
#include "../../event/signals.hpp"

#include "../../math/math.hpp"
#include <iostream>
#include <vector>
#include <map>

// Forward declarations
namespace Engine
{
	class EnoEngine;
}

namespace ECS
{
	class Entity;
	class EntityManager;
}

namespace Editor
{
	struct Node;
	class EnoSceneTree;
}

namespace ECSComponents
{
	struct CanvasComponent
	{
		// Default constructor
		CanvasComponent() = default;

		// Constructor with viewport size
		CanvasComponent(float width, float height) : viewportWidth(width), viewportHeight(height) {}
		// Canvas viewport dimensions (1920 x 1080 by default)
		float viewportWidth{ 1920.0f };
		float viewportHeight{ 1080.0f };

		// Canvas-specific layer (1-5, separate from global z_index 0-19)
		uint32_t canvasLayer{ 3 };

		// Whether canvas follows camera for HUD behavior
		bool followCamera{ true };

		// Scene node ID of the camera to follow (-1 means no specific camera selected)
		uint32_t followCameraEntityID{ static_cast<uint32_t>(-1) };

		// Offset when following camera (for positioning like top-left, top-right, etc.)
		Math::eVec2<float> cameraOffset{ 0.0f, 0.0f };

		// Whether this canvas is active
		bool isActive{ true };

		// Accessors
		const float& getViewPortWidth() const
		{
			return viewportWidth;
		}

		const float& getViewPortHeight() const
		{
			return viewportHeight;
		}

		const bool& getActive() const
		{
			return isActive;
		}

		const bool& getFollowCamera() const
		{

			return followCamera;
		}



		// Mutators
		void setViewPortWidth(float w)
		{
			viewportWidth = w;
		}

		void setViewPortHeight(float h)
		{
			viewportHeight = h;
		}

		void setActive(bool flag)
		{
			isActive = flag;
		}

		void setFollowCamera(bool flag)
		{
			followCamera = flag;
		}

		// Signals
		std::unordered_map<std::string, Events::EnoSignal> signals_;

		// For serialization
		template <typename Visitor>
		void reflect(Visitor&& l)
		{
			l("Viewport Width",			viewportWidth,			"float");
			l("Viewport Height",		viewportHeight,			"float");
			l("Canvas Layer",			canvasLayer,			"uint32_t");
			l("Follow Camera",			followCamera,			"bool");
			l("Follow Camera Entity",	followCameraEntityID,	"uint32_t");
			l("Is Active",				isActive,				"bool");
		}

		template <typename Visitor>
		void reflect(Visitor&& l) const
		{
			l("Viewport Width",			viewportWidth,			"float");
			l("Viewport Height",		viewportHeight,			"float");
			l("Canvas Layer",			canvasLayer,			"uint32_t");
			l("Follow Camera",			followCamera,			"bool");
			l("Follow Camera Entity",	followCameraEntityID,	"uint32_t");
			l("Is Active",				isActive,				"bool");
		}
	};

	// Print overload
	inline std::ostream& operator<<(std::ostream& os, const CanvasComponent& c)
	{
		os << "Viewport: " << c.viewportWidth << "x" << c.viewportHeight
		   << " | Layer: " << c.canvasLayer
		   << " | Follow Camera: " << (c.followCamera ? "Yes" : "No")
		   << " | Follow Camera Entity: " << c.followCameraEntityID
		   << " | Active: " << (c.isActive ? "Yes" : "No")
		   << std::endl;
		return os;
	}

	/*============================================================================================
	 * \brief				Check if entity or any ancestor has CanvasComponent
	 * \param[in]			entityID - the ID of the entity to check
	 * \param[in]			tree - reference to the scene tree
	 * \param[in]			entityMng - reference to EntityManager
	 * \return				true if entity or ancestor has CanvasComponent, false otherwise
	*//*========================================================================================*/
	bool hasCanvasAncestor(uint32_t entityID, Editor::EnoSceneTree& tree, ECS::EntityManager& entityMng);

	/*============================================================================================
	 * \brief				Collect canvas children from scene tree (like Godot)
	 * \param[in]			engine - Reference to EnoEngine
	 * \param[in]			canvasEntity - Canvas entity whose children to collect
	 * \param[out]			layers - Map to populate with entities sorted by canvas layer (1-5)
	*//*========================================================================================*/
	void collectCanvasChildren(Engine::EnoEngine& engine, ECS::Entity& canvasEntity, std::map<int, std::vector<ECS::Entity*>>& layers);

	/*============================================================================================
	 * \brief				Collect text entities that are canvas children (from scene tree like Godot)
	 * \param[in]			engine - Reference to EnoEngine
	 * \param[in]			canvasEntity - Canvas entity whose text children to collect
	 * \param[out]			textEntities - Vector to populate with text entities
	*//*========================================================================================*/
	void collectCanvasTextEntities(Engine::EnoEngine& engine, ECS::Entity& canvasEntity, std::vector<ECS::Entity*>& textEntities);

	/*============================================================================================
	 * \brief				Render canvas entities (shapes, sprites) with layering
	 * \param[in]			engine - Reference to EnoEngine
	 * \param[in]			cachedEntities - Vector of cached drawable entities
	 * \param[in]			canvasEntity - Canvas entity to render
	 * \param[in]			canvas - CanvasComponent with rendering settings
	 * \param[in]			cameraPos - Current camera position for offset calculation
	*//*========================================================================================*/
	void renderCanvasEntities(Engine::EnoEngine& engine, std::vector<ECS::Entity>& cachedEntities, ECS::Entity& canvasEntity, const CanvasComponent& canvas, const Math::eVec2<float>& cameraTopLeft);

	/*============================================================================================
	 * \brief				Render text entities on canvas
	 * \param[in]			engine - Reference to EnoEngine
	 * \param[in]			cachedEntities - Vector of cached drawable entities
	 * \param[in]			canvasEntity - Canvas entity to render text for
	 * \param[in]			canvas - CanvasComponent with rendering settings
	 * \param[in]			cameraPos - Current camera position for offset calculation
	*//*========================================================================================*/
	void renderCanvasText(Engine::EnoEngine& engine, std::vector<ECS::Entity>& cachedEntities, ECS::Entity& canvasEntity, const CanvasComponent& canvas, const Math::eVec2<float>& cameraTopLeft);

	/*============================================================================================
	 * \brief				Render all active canvas entities in the scene
	 * \param[in]			engine - Reference to EnoEngine
	 * \param[in]			cachedEntities - Vector of cached drawable entities
	 * \param[in]			cameraPos - Current camera position for offset calculation
	*//*========================================================================================*/
	void renderAllCanvases(Engine::EnoEngine& engine, std::vector<ECS::Entity>& cachedEntities, const Math::eVec2<float>& cameraPos);
}
