/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         canvas_system.cpp
* @author       Caven Koh (caven.koh@digipen.edu)   60%
* @co-author    Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   40%
* @brief        Implementation of CanvasSystem — canvas rendering and hierarchy traversal.
*//*========================================================================================*/


#include "canvas_system.hpp"

#include "../ecs/entity.hpp"
#include "../graphics/graphics.hpp"
#include "../engine/eno_engine.hpp"
#include "../editor/eno_scene_tree.hpp"
#include "../component/components/canvas.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/visibility.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/text_component.hpp"
#include "../component/components/camera_component.hpp"
#include "../component/components/name.hpp"

#include <algorithm>
#include <iostream>

namespace UI
{
	// -------------------------------------------------------------------------
	// Static helpers
	// -------------------------------------------------------------------------

	bool CanvasSystem::hasCanvasAncestor(uint32_t entityID, Editor::EnoSceneTree& tree, ECS::EntityManager& entityMng)
	{
		Editor::Node* node = tree.getNodeByEntityID(entityID);
		if (!node) return false;

		// Travel up the node hierachy -> root
		Editor::Node* current = node->parent_;
		while (current) {
			if (current->entity_.has<ECSComponents::CanvasComponent>(entityMng)) {
				return true;
			}
			current = current->parent_;
		}
		return false;
	}

	void CanvasSystem::collectCanvasChildrenRecursive(
		Editor::Node* node,
		ECS::EntityManager& entityMng,
		std::map<int, std::vector<ECS::Entity*>>& layers)
	{
		// Check valid node
		if (!node) return;
		
		// Check for visibility component and status
		if (node->entity_.has<ECSComponents::VisibilityComponent>(entityMng)) {
			auto& vis = node->entity_.get<ECSComponents::VisibilityComponent>(entityMng);
			if (!vis.visible) return;
		}

		// Clamp visbility layer to be between 1 - 5 for canvas layering
		if (node->entity_.has<ECSComponents::DrawInfoComponent>(entityMng)) {
			if (node->entity_.has<ECSComponents::VisibilityComponent>(entityMng)) {
				auto& vis = node->entity_.get<ECSComponents::VisibilityComponent>(entityMng);
				int canvasLayer = static_cast<int>(std::clamp(vis.z_index, 1u, 5u));
				// Add entity to map
				layers[canvasLayer].push_back(&node->entity_);
			}
			else {
				// Set to the middle layer by default
				layers[3].push_back(&node->entity_);
			}
		}

		// Recurvsively collects all children of the current node
		for (auto* child : node->children_) {
			collectCanvasChildrenRecursive(child, entityMng, layers);
		}
	}

	void CanvasSystem::collectCanvasChildren(
		Engine::EnoEngine& engine,
		ECS::Entity& canvasEntity,
		std::map<int, std::vector<ECS::Entity*>>& layers)
	{
		// Clear the map before start collecting
		layers.clear();

		auto& entityMng  = engine.getEntityManager();
		auto& sceneTree  = engine.getEnoEditor().getSceneTree();

		Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
		if (!canvasNode) {
			// Canvas node not found
			return;
		}
		
		// Collect all children into the map with recursive function
		for (auto* child : canvasNode->children_) {
			collectCanvasChildrenRecursive(child, entityMng, layers);
		}
	}

	void CanvasSystem::collectCanvasTextRecursive(
		Editor::Node* node,
		ECS::EntityManager& entityMng,
		std::vector<ECS::Entity*>& textEntities)
	{
		// Check valid node
		if (!node) return;

		// Check for visibility component and status
		if (node->entity_.has<ECSComponents::VisibilityComponent>(entityMng)) {
			auto& vis = node->entity_.get<ECSComponents::VisibilityComponent>(entityMng);
			if (!vis.visible) return;
		}

		// Add entity to map
		if (node->entity_.has<ECSComponents::TextComponent>(entityMng)) {
			textEntities.push_back(&node->entity_);
		}

		// Recurvsively collects all children of the current node
		for (auto* child : node->children_) {
			collectCanvasTextRecursive(child, entityMng, textEntities);
		}
	}

	void CanvasSystem::collectCanvasTextEntities(
		Engine::EnoEngine& engine,
		ECS::Entity& canvasEntity,
		std::vector<ECS::Entity*>& textEntities)
	{
		// Clear the map
		textEntities.clear();

		auto& entityMng  = engine.getEntityManager();
		auto& sceneTree  = engine.getEnoEditor().getSceneTree();

		Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
		if (!canvasNode) return;

		// Collect all children into the map with recursive function
		for (auto* child : canvasNode->children_) {
			collectCanvasTextRecursive(child, entityMng, textEntities);
		}
	}

	// -------------------------------------------------------------------------
	// Instance methods
	// -------------------------------------------------------------------------

	void CanvasSystem::renderCanvasEntities(
		Engine::EnoEngine& engine,
		std::vector<ECS::Entity>& /*cachedEntities*/,
		ECS::Entity& canvasEntity,
		const Math::eVec2<float>& cameraTopLeft)
	{
		auto& graphics       = engine.getGraphicsManager();
		auto& entityManager  = engine.getEntityManager();

		// Collect all children of current canvas into layering map
		std::map<int, std::vector<ECS::Entity*>> canvasLayers;
		collectCanvasChildren(engine, canvasEntity, canvasLayers);

#if 0
		// Debugger to check all canvas entities
		// Loop through each layer
		for (auto& [lyr, ents] : canvasLayers) {
			// Loop through each entity
			for (auto* ep : ents) {
				const char* nm = ep->has<ECSComponents::NameComponent>(entityManager)
					? ep->get<ECSComponents::NameComponent>(entityManager).name : "(no name)";
				std::cout << "[CANVAS DBG]   Child: '" << nm << "' id=" << ep->id() << "\n";
			}
		}
#endif
		// Loop through each layer
		for (auto& [layer, entities] : canvasLayers) {
			// Loop through each entity
			for (ECS::Entity* entityPtr : entities) {
				ECS::Entity& entity = *entityPtr;
				// Check for render component
				try {
					if (!entity.has<ECSComponents::DrawInfoComponent>(entityManager)) continue;

					// Render entity after converting from world to screen space
					Math::eVec2<float> worldPos = entity.get<ECSComponents::TransformComponent>(entityManager).pos;
					auto& renderInfo = entity.get<ECSComponents::DrawInfoComponent>(entityManager);

					Math::eVec2<float> renderPos = {
						worldPos.x - cameraTopLeft.x,
						worldPos.y - cameraTopLeft.y
					};

					auto& tc = entity.get<ECSComponents::TransformComponent>(entityManager);
					float scaledWidth  = renderInfo.width  * tc.scale.x;
					float scaledHeight = renderInfo.height * tc.scale.y;
					float rotation     = tc.rot.x;

					if (renderInfo.texture && renderInfo.texture->isValid()) {
						Math::eVec2<float> topLeft = {
							renderPos.x - (scaledWidth  * 0.5f),
							renderPos.y - (scaledHeight * 0.5f)
						};

						Math::eVec2<float> uvStart{ 0.0f, 0.0f };
						Math::eVec2<float> uvEnd  { 1.0f, 1.0f };

						if (entity.has<ECSComponents::AnimationControllerComponent>(entityManager)) {
							auto& animComp = entity.get<ECSComponents::AnimationControllerComponent>(entityManager);
							Animation::AnimationController* ctrl =
								engine.getAnimationRegistry().get(animComp.controllerID);
							if (ctrl) {
								const auto& frame = ctrl->getCurrentFrame();
								uvStart = frame.uvStart;
								uvEnd   = frame.uvEnd;
							}
						}

						graphics.drawTexturedQuadUV(
							topLeft,
							Math::eVec2<float>{ scaledWidth, scaledHeight },
							renderInfo.texture,
							uvStart, uvEnd,
							renderInfo.color,
							rotation);
					}
					else if (strcmp(renderInfo.shape, "Circle") == 0) {
						float scaledRadius = scaledWidth * 0.5f;
						graphics.drawCircle(renderPos, scaledRadius, renderInfo.color, true, 32);
					}
					else {
						float topLeftX = renderPos.x - (scaledWidth  * 0.5f);
						float topLeftY = renderPos.y - (scaledHeight * 0.5f);
						graphics.drawRect(
							Math::eVec2<float>{ topLeftX, topLeftY },
							Math::eVec2<float>{ scaledWidth, scaledHeight },
							renderInfo.color, true, rotation);
					}
				}
				catch (...) { continue; }
			}
		}

		graphics.flush();
	}

	void CanvasSystem::renderCanvasText(
		Engine::EnoEngine& engine,
		std::vector<ECS::Entity>& /*cachedEntities*/,
		ECS::Entity& canvasEntity,
		const Math::eVec2<float>& cameraTopLeft)
	{
		auto& graphics      = engine.getGraphicsManager();
		auto& entityManager = engine.getEntityManager();

		// Resolve viewport dimensions from the active camera (needed by drawTextScreen)
		float viewportW = 1920.f, viewportH = 1080.f;
		{
			auto cameraEntities = entityManager.findEntity<ECSComponents::CameraComponent>();
			for (auto& ce : cameraEntities)
			{
				auto& cam = ce.get<ECSComponents::CameraComponent>(entityManager);
				if (!cam.isActive) continue;
				viewportW = cam.viewportWidth;
				viewportH = cam.viewportHeight;
				break;
			}
		}

		// Collect all text children of this canvas
		std::vector<ECS::Entity*> textEntities;
		collectCanvasTextEntities(engine, canvasEntity, textEntities);

		// Flush pending batch before switching to text shader
		graphics.flush();

		// Loop through each entity
		for (auto* entityPtr : textEntities) {
			ECS::Entity& textEntity = *entityPtr;
			try {
				// Check for text and transform component
				if (!textEntity.has<ECSComponents::TextComponent>(entityManager))      continue;
				if (!textEntity.has<ECSComponents::TransformComponent>(entityManager)) continue;

				// Get text
				auto& textComp = textEntity.get<ECSComponents::TextComponent>(entityManager);
				if (strlen(textComp.text) == 0) continue;

				// Select font
				auto font = engine.getResourceLoader().getFont(std::string(textComp.fontName));
				if (!font) continue;

				// Convert world position to screen-relative coordinates, matching how
				// non-canvas text entities are rendered via drawTextScreen in graphics.cpp
				Math::eVec2<float> worldPos  = textEntity.get<ECSComponents::TransformComponent>(entityManager).pos;
				Math::eVec2<float> screenPos = { worldPos.x - cameraTopLeft.x,
				                                 worldPos.y - cameraTopLeft.y };

				std::string fullText(textComp.text);
				float  lineHeight = 60.0f * textComp.fontSize;
				float  yOffset    = 0.0f;
				size_t start      = 0;

				while (start <= fullText.size()) {
					size_t pos  = fullText.find('\n', start);
					std::string line = (pos != std::string::npos)
						? fullText.substr(start, pos - start)
						: fullText.substr(start);

					if (!line.empty()) {
						graphics.drawTextScreen(
							font, line,
							Math::eVec2<float>{ screenPos.x, screenPos.y + yOffset },
							textComp.fontSize,
							Graphics::Color::White,
							viewportW, viewportH);
					}

					if (pos == std::string::npos) break;
					yOffset += lineHeight;
					start = pos + 1;
				}
			}
			catch (...) { continue; }
		}

		graphics.rebindDefaultShader();
	}

	void CanvasSystem::update(Engine::EnoEngine& engine)
	{
		auto& entityManager = engine.getEntityManager();

		// Resolve the fallback camera position once for canvases that don't specify a camera.
		Math::eVec2<float> defaultCameraPos{ 0.0f, 0.0f };
		for (auto& camEnt : entityManager.findEntity<ECSComponents::CameraComponent>())
		{
			auto& cam = camEnt.get<ECSComponents::CameraComponent>(entityManager);
			if (!cam.isActive) continue;
			defaultCameraPos = cam.position;
			break;
		}

		for (ECS::Entity& canvasEntity : entityManager.findEntity<ECSComponents::CanvasComponent>())
		{
			try {
				auto& canvas = canvasEntity.get<ECSComponents::CanvasComponent>(entityManager);

				// Inactive canvases: restore their transform to the pre-follow position.
				if (!canvas.isActive && engine.getIsPlaying())
				{
					if (canvas.hasOriginalPos)
					{
						auto& sceneTree      = engine.getEnoEditor().getSceneTree();
						Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
						ECSComponents::moveNodeWithChildren(canvasNode, canvas.originalPos, entityManager);
						canvas.hasOriginalPos = false;
					}
					continue;
				}

				// Physically move canvas and its children to follow the camera.
				// Only runs when a valid camera is assigned (followCameraEntityID != -1).
				if (canvas.followCamera && engine.getIsPlaying() &&
				    canvas.followCameraEntityID != static_cast<uint32_t>(-1))
				{
					// Save the original position on the first frame before moving.
					if (!canvas.hasOriginalPos && canvasEntity.has<ECSComponents::TransformComponent>(entityManager))
					{
						canvas.originalPos    = canvasEntity.get<ECSComponents::TransformComponent>(entityManager).pos;
						canvas.hasOriginalPos = true;
					}

					// Use the canvas's assigned camera if available, otherwise fall back.
					Math::eVec2<float> resolvedCameraPos = defaultCameraPos;
					auto& sceneTree = engine.getEnoEditor().getSceneTree();
					Editor::Node* camNode = sceneTree.getNodeBySceneObjectID(canvas.followCameraEntityID);
					if (camNode && camNode->entity_.has<ECSComponents::CameraComponent>(entityManager))
						resolvedCameraPos = camNode->entity_.get<ECSComponents::CameraComponent>(entityManager).position;

					Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
					Math::eVec2<float> targetPos = {
						resolvedCameraPos.x + canvas.cameraOffset.x,
						resolvedCameraPos.y + canvas.cameraOffset.y
					};
					ECSComponents::moveNodeWithChildren(canvasNode, targetPos, entityManager);
				}
			}
			catch (...) { continue; }
		}
	}

	void CanvasSystem::render(
		Engine::EnoEngine& engine,
		std::vector<ECS::Entity>& cachedEntities,
		const Math::eVec2<float>& cameraPos)
	{
		auto& entityManager = engine.getEntityManager();

		for (ECS::Entity& canvasEntity : cachedEntities) {
			if (!canvasEntity.has<ECSComponents::CanvasComponent>(entityManager)) continue;
			try {
				auto& canvas = canvasEntity.get<ECSComponents::CanvasComponent>(entityManager);

				// In play mode, inactive canvases are hidden. In the editor, always render.
				if (!canvas.isActive && engine.getIsPlaying())
					continue;

				renderCanvasEntities(engine, cachedEntities, canvasEntity, { 0.0f, 0.0f });
				renderCanvasText(engine, cachedEntities, canvasEntity, cameraPos);
			}
			catch (...) { continue; }
		}
	}

} // namespace UI
