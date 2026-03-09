/*============================================================================================
* @file         canvas.cpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         27/02/2026
* @brief        This file contains the definition of the canvas component class for UI elements
*//*========================================================================================*/

#include "canvas.hpp"
#include "../../engine/eno_engine.hpp"
#include "../../editor/eno_scene_tree.hpp"
#include "transform.hpp"
#include "visibility.hpp"
#include "drawInfo.hpp"
#include "text_component.hpp"
#include "camera_component.hpp"
#include "name.hpp"
#include <algorithm>

// External function for marking entities dirty
extern void markGameEntitiesDirty();

namespace ECSComponents
{
	// Check if any parent node has CanvasComponent
	bool hasCanvasAncestor(uint32_t entityID, Editor::EnoSceneTree& tree, ECS::EntityManager& entityMng)
	{
		Editor::Node* node = tree.getNodeByEntityID(entityID);
		if (!node) return false;

		// Walk up parent chain
		Editor::Node* current = node->parent_;
		while (current) {
			if (current->entity_.has<CanvasComponent>(entityMng)) {
				return true;
			}
			current = current->parent_;
		}
		return false;
	}

	// Recursively collect children of a canvas node
	void collectCanvasChildrenRecursive(
		Editor::Node* node,
		ECS::EntityManager& entityMng,
		std::map<int, std::vector<ECS::Entity*>>& layers
	) {
		if (!node) return;

		// Check visibility
		if (node->entity_.has<VisibilityComponent>(entityMng)) {
			auto& vis = node->entity_.get<VisibilityComponent>(entityMng);

			// Skip invisible entities
			if (!vis.visible) {
				return; // Don't process this node or its children
			}
		}

		// Add this entity to appropriate layer if it has DrawInfoComponent
		if (node->entity_.has<DrawInfoComponent>(entityMng)) {
			if (node->entity_.has<VisibilityComponent>(entityMng)) {
				auto& vis = node->entity_.get<VisibilityComponent>(entityMng);
				// Map z_index to canvas layer (1-5)
				int canvasLayer = static_cast<int>(std::clamp(vis.z_index, 1u, 5u));
				//std::cout << "[CANVAS LAYER] " << canvasLayer << std::endl;
				layers[canvasLayer].push_back(&node->entity_);
			}
			else {
				// Entities without VisibilityComponent go to default layer 3
				layers[3].push_back(&node->entity_);
			}
		}

		// Recursively process all children
		for (auto* child : node->children_) {
			collectCanvasChildrenRecursive(child, entityMng, layers);
		}
	}

	// Collect canvas children from scene tree (like Godot)
	void collectCanvasChildren(
		Engine::EnoEngine& engine,
		ECS::Entity& canvasEntity,
		std::map<int, std::vector<ECS::Entity*>>& layers
	) {
		layers.clear();

		auto& entityMng = engine.getEntityManager();
		auto& sceneTree = engine.getEnoEditor().getSceneTree();

		// Get the canvas entity's node in the scene tree
		Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
		if (!canvasNode) {
			std::cout << "[CANVAS DBG] WARNING: canvas node not found in scene tree, entityID=" << canvasEntity.id() << "\n";
			return;
		}
		std::cout << "[CANVAS DBG] Canvas node found, direct child count=" << canvasNode->children_.size() << "\n";

		// Recursively collect all children under this canvas
		for (auto* child : canvasNode->children_) {
			collectCanvasChildrenRecursive(child, entityMng, layers);
		}
	}

	// Recursively collect text entities from canvas children
	void collectCanvasTextRecursive(
		Editor::Node* node,
		ECS::EntityManager& entityMng,
		std::vector<ECS::Entity*>& textEntities
	) {
		if (!node) return;

		// Check visibility
		if (node->entity_.has<VisibilityComponent>(entityMng)) {
			auto& vis = node->entity_.get<VisibilityComponent>(entityMng);
			if (!vis.visible) {
				return; // Don't process this node or its children
			}
		}

		// Add to list if it has TextComponent
		if (node->entity_.has<TextComponent>(entityMng)) {
			textEntities.push_back(&node->entity_);
		}

		// Recursively process all children
		for (auto* child : node->children_) {
			collectCanvasTextRecursive(child, entityMng, textEntities);
		}
	}

	// Collect text entities that are canvas children (from scene tree like Godot)
	void collectCanvasTextEntities(
		Engine::EnoEngine& engine,
		ECS::Entity& canvasEntity,
		std::vector<ECS::Entity*>& textEntities
	) {
		textEntities.clear();

		auto& entityMng = engine.getEntityManager();
		auto& sceneTree = engine.getEnoEditor().getSceneTree();

		// Get the canvas entity's node in the scene tree
		Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
		if (!canvasNode) return;

		// Recursively collect all text children under this canvas
		for (auto* child : canvasNode->children_) {
			collectCanvasTextRecursive(child, entityMng, textEntities);
		}
	}

	// Render canvas entities with relative to the camera in world space
	void renderCanvasEntities(
		Engine::EnoEngine& engine,
		std::vector<ECS::Entity>& cachedEntities,
		ECS::Entity& canvasEntity,
		const CanvasComponent& canvas,
		const Math::eVec2<float>& cameraTopLeft
	) {
		auto& graphics = engine.getGraphicsManager();
		auto& entityManager = engine.getEntityManager();

		// Collect canvas children into layers (1-5) from scene tree
		std::map<int, std::vector<ECS::Entity*>> canvasLayers;
		collectCanvasChildren(engine, canvasEntity, canvasLayers);

		// Debug: log canvas world pos + layers
		std::cout << "[CANVAS DBG] Layers=" << canvasLayers.size() << "\n";
		for (auto& [lyr, ents] : canvasLayers) {
			std::cout << "[CANVAS DBG] Layer " << lyr << ": " << ents.size() << " entities\n";
			for (auto* ep : ents) {
				const char* nm = ep->has<NameComponent>(entityManager) ? ep->get<NameComponent>(entityManager).name : "(no name)";
				std::cout << "[CANVAS DBG]   Child: '" << nm << "' id=" << ep->id() << "\n";
			}
		}

		// Render each canvas layer
		for (auto& [layer, entities] : canvasLayers) {
			for (ECS::Entity* entityPtr : entities) {
				ECS::Entity& entity = *entityPtr;

				try {
					// Skip entities without DrawInfoComponent
					if (!entity.has<DrawInfoComponent>(entityManager)) {
						continue;
					}

					Math::eVec2<float> worldPos = entity.get<TransformComponent>(entityManager).pos;
					auto& renderInfo = entity.get<DrawInfoComponent>(entityManager);

					Math::eVec2<float> renderPos = {
						worldPos.x - cameraTopLeft.x,
						worldPos.y - cameraTopLeft.y
					};

					// Debug: per-entity render info
					{
						const char* entName = entity.has<NameComponent>(entityManager) ? entity.get<NameComponent>(entityManager).name : "(no name)";
						std::cout << "[CANVAS DBG] Draw '" << entName << "' renderPos=(" << renderPos.x << ", " << renderPos.y << ")" << "\n";
					}

					// Calculate scaled dimensions and rotation
					auto& tc = entity.get<TransformComponent>(entityManager);
					float scaledWidth = renderInfo.width * tc.scale.x;
					float scaledHeight = renderInfo.height * tc.scale.y;
					float rotation = tc.rot.x;

					// Render textured quad
					if (renderInfo.texture && renderInfo.texture->isValid()) {
						Math::eVec2<float> topLeft = {
							renderPos.x - (scaledWidth * 0.5f),
							renderPos.y - (scaledHeight * 0.5f)
						};

						Math::eVec2<float> uvStart{ 0.0f, 0.0f };
						Math::eVec2<float> uvEnd{ 1.0f, 1.0f };

						// Check for animation component
						if (entity.has<AnimationControllerComponent>(entityManager)) {
							auto& animComp = entity.get<AnimationControllerComponent>(entityManager);
							Animation::AnimationController* ctrl =
								engine.getAnimationRegistry().get(animComp.controllerID);

							if (ctrl) {
								const auto& frame = ctrl->getCurrentFrame();
								uvStart = frame.uvStart;
								uvEnd = frame.uvEnd;
							}
						}

						graphics.drawTexturedQuadUV(
							topLeft,
							Math::eVec2<float>{scaledWidth, scaledHeight},
							renderInfo.texture,
							uvStart,
							uvEnd,
							renderInfo.color,
							rotation
						);
					}
					// Render circle
					else if (strcmp(renderInfo.shape, "Circle") == 0) {
						float scaledRadius = (scaledWidth * 0.5f);
						graphics.drawCircle(renderPos, scaledRadius, renderInfo.color, true, 32);
					}
					// Render rectangle (default)
					else {
						float topLeftX = renderPos.x - (scaledWidth * 0.5f);
						float topLeftY = renderPos.y - (scaledHeight * 0.5f);
						graphics.drawRect(
							Math::eVec2<float>{topLeftX, topLeftY},
							Math::eVec2<float>{scaledWidth, scaledHeight},
							renderInfo.color,
							true,
							rotation
						);
					}
				}
				catch (...) 
				{
					markGameEntitiesDirty();
					continue;
				}
			}
		}

		// Flush the batch after rendering all shapes
		graphics.flush();
	}

	// Render text entities on canvas
	void renderCanvasText(
		Engine::EnoEngine& engine,
		std::vector<ECS::Entity>& cachedEntities,
		ECS::Entity& canvasEntity,
		const CanvasComponent& canvas,
		const Math::eVec2<float>& cameraTopLeft
	) {
		auto& graphics = engine.getGraphicsManager();
		auto& entityManager = engine.getEntityManager();

		// Find all TextComponent entities under this canvas
		std::vector<ECS::Entity*> textEntities;
		collectCanvasTextEntities(engine, canvasEntity, textEntities);

		// Debug: log text entities found
		std::cout << "[CANVAS DBG] Text entities under canvas: " << textEntities.size() << "\n";
		for (auto* ep : textEntities) {
			const char* nm = ep->has<NameComponent>(entityManager) ? ep->get<NameComponent>(entityManager).name : "(no name)";
			std::cout << "[CANVAS DBG]   Text child: '" << nm << "'\n";
		}

		auto& windowMng = engine.getWindowManager();
		auto fbSize = windowMng.getFramebufferSize();

		for (auto* entityPtr : textEntities) {
			ECS::Entity& textEntity = *entityPtr;

			try {
				if (!textEntity.has<TextComponent>(entityManager)) continue;
				if (!textEntity.has<TransformComponent>(entityManager)) continue;

				// Check visibility
				if (textEntity.has<VisibilityComponent>(entityManager)) {
					if (!textEntity.get<VisibilityComponent>(entityManager).visible) {
						continue;
					}
				}

				auto& textComp = textEntity.get<TextComponent>(entityManager);

				if (strlen(textComp.text) == 0) continue;

				auto font = engine.getResourceLoader().getFont(std::string(textComp.fontName));
				if (!font) continue;

				Math::eVec2<float> worldPos = textEntity.get<TransformComponent>(entityManager).pos;
				Math::eVec2<float> screenPos = {
					worldPos.x - cameraTopLeft.x,
					worldPos.y - cameraTopLeft.y
				};

				// Debug: per-text-entity render info
				{
					const char* tnm = textEntity.has<NameComponent>(entityManager) ? textEntity.get<NameComponent>(entityManager).name : "(no name)";
					std::cout << "[CANVAS DBG] Text '" << tnm << "' screenPos=(" << screenPos.x << ", " << screenPos.y << ") fontSize=" << textComp.fontSize << " text='" << textComp.text << "'\n";
				}

				// Draw text using drawTextScreen (multiline support)
				std::string fullText(textComp.text);
				float lineHeight = 60.0f * textComp.fontSize;
				float yOffset = 0.0f;
				size_t start = 0;

				while (start <= fullText.size()) {
					size_t pos = fullText.find('\n', start);
					std::string line = (pos != std::string::npos)
						? fullText.substr(start, pos - start)
						: fullText.substr(start);

					if (!line.empty()) {
						graphics.drawTextScreen(
							font, line,
							Math::eVec2<float>{ screenPos.x, screenPos.y + yOffset },
							textComp.fontSize,
							Graphics::Color::White,
							static_cast<float>(fbSize.first),
							static_cast<float>(fbSize.second)
						);
					}

					if (pos == std::string::npos) break;
					yOffset += lineHeight;
					start = pos + 1;
				}
			}
			catch (...)
			{
				markGameEntitiesDirty();
				continue;
			}
		}

		// Restore default shader after text rendering
		graphics.rebindDefaultShader();
	}

	// Render all active canvas entities in the scene
	void renderAllCanvases(Engine::EnoEngine& engine, std::vector<ECS::Entity>& cachedEntities, const Math::eVec2<float>& cameraPos)
	{
		auto& entityManager = engine.getEntityManager();

		// Canvas entities have DrawInfoComponent so they are already in cachedEntities
		for (ECS::Entity& canvasEntity : cachedEntities) {
			if (!canvasEntity.has<CanvasComponent>(entityManager)) continue;
			try {
				auto& canvas = canvasEntity.get<CanvasComponent>(entityManager);

				// Debug: log canvas found
				{
					const char* cnvName = canvasEntity.has<NameComponent>(entityManager) ? canvasEntity.get<NameComponent>(entityManager).name : "(no name)";
					std::cout << "[CANVAS DBG] Canvas='" << cnvName << "' id=" << canvasEntity.id() << " active=" << canvas.isActive << " followCam=" << canvas.followCamera << " followCamID=" << canvas.followCameraEntityID << "\n";
				}

				if (!canvas.isActive) {
					continue;
				}

				// Resolve which camera position to use for this canvas (play mode only)
				Math::eVec2<float> resolvedCameraPos = cameraPos;
				if (canvas.followCamera && engine.getIsPlaying() && canvas.followCameraEntityID != static_cast<uint32_t>(-1)) {
					auto& sceneTree = engine.getEnoEditor().getSceneTree();
					Editor::Node* camNode = sceneTree.getNodeBySceneObjectID(canvas.followCameraEntityID);
					if (camNode && camNode->entity_.has<CameraComponent>(entityManager)) {
						resolvedCameraPos = camNode->entity_.get<CameraComponent>(entityManager).position;
					}
				}

				std::cout << "[CANVAS DBG] Resolved camera pos: (" << resolvedCameraPos.x << ", " << resolvedCameraPos.y << ")\n";

				// Physically move canvas node (and all children) to camera-relative position
				if (canvas.followCamera && engine.getIsPlaying()) {
					auto& sceneTree = engine.getEnoEditor().getSceneTree();
					Editor::Node* canvasNode = sceneTree.getNodeByEntityID(canvasEntity.id());
					Math::eVec2<float> targetPos = {
						resolvedCameraPos.x + canvas.cameraOffset.x,
						resolvedCameraPos.y + canvas.cameraOffset.y
					};
					ECSComponents::moveNodeWithChildren(canvasNode, targetPos, entityManager);
				}

				// Render canvas entities and text with the correct coordinate offsets.
				// Editor mode: projection maps [0, viewportSize] -> NDC; subtract camera
				//   top-left so world coords become screen-pixel coords.
				// Play mode: projection is world-space (camera-relative orthographic);
				//   shapes must be drawn at their world positions (no manual subtraction).
				//   drawTextScreen uses its own screen-space projection [0, fbW], so text
				//   needs worldPos converted to screen coords via camTopLeft.
				if (engine.getIsPlaying()) {
					// Shapes: world-space projection handles the transform, pass zero offset
					renderCanvasEntities(engine, cachedEntities, canvasEntity, canvas,
						Math::eVec2<float>{ 0.0f, 0.0f });

					// Text: drawTextScreen expects screen-space [0, fbW] coords;
					// convert by subtracting the camera's top-left corner
					Math::eVec2<float> camTopLeft = {
						resolvedCameraPos.x - canvas.viewportWidth  * 0.5f,
						resolvedCameraPos.y - canvas.viewportHeight * 0.5f
					};
					renderCanvasText(engine, cachedEntities, canvasEntity, canvas, camTopLeft);
				}
				else {
					// Editor: resolvedCameraPos is already the camera top-left (editorCameraX/Y)
					renderCanvasEntities(engine, cachedEntities, canvasEntity, canvas, resolvedCameraPos);
					renderCanvasText(engine, cachedEntities, canvasEntity, canvas, resolvedCameraPos);
				}
			}
			catch (...)
			{
				markGameEntitiesDirty();
				continue;
			}
		}
	}
}
