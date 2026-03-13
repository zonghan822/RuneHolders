/*============================================================================================
* @file         canvas_system.hpp
* @brief        Declaration of CanvasSystem — processes and renders all active canvas entities.
*//*========================================================================================*/
#pragma once

#include "../math/math.hpp"
#include <vector>
#include <map>

// Forward Declarations
namespace Engine { class EnoEngine; }
namespace ECS    { class Entity; class EntityManager; }
namespace Editor { struct Node; class EnoSceneTree; }

namespace UI
{
	/*============================================================================================
	* \class	CanvasSystem
	* \brief	Owns all canvas rendering and hierarchy traversal logic.
	*//*========================================================================================*/
	class CanvasSystem
	{
	public:
		/*============================================================================================
		* \brief		Render all active canvas entities on top of world content.
		* \param[in]	engine          Reference to EnoEngine
		* \param[in]	cachedEntities  Pre-sorted drawable entity list
		* \param[in]	cameraPos       Camera top-left position in world space
		*//*========================================================================================*/
		void render(Engine::EnoEngine&        engine,
		            std::vector<ECS::Entity>& cachedEntities,
		            const Math::eVec2<float>& cameraPos);

		/*============================================================================================
		* \brief		Check if an entity or any of its ancestors has a CanvasComponent.
		* \param[in]	entityID   ID of the entity to check
		* \param[in]	tree       Scene tree reference
		* \param[in]	entityMng  Entity manager reference
		* \return		true if entity or ancestor has CanvasComponent
		*//*========================================================================================*/
		static bool hasCanvasAncestor(uint32_t          entityID,
		                              Editor::EnoSceneTree& tree,
		                              ECS::EntityManager&   entityMng);

	private:
		/*============================================================================================
		* \brief		Recursively walk a scene node and bucket all drawable (DrawInfoComponent)
		*				descendants into the provided layer map, skipping invisible nodes.
		* \param[in]	node       Root of the sub-tree to traverse
		* \param[in]	entityMng  Entity manager used to query components
		* \param[out]	layers     Map of canvas layer index → list of drawable entity pointers
		*//*========================================================================================*/
		static void collectCanvasChildrenRecursive(
			Editor::Node* node,
			ECS::EntityManager& entityMng,
			std::map<int, std::vector<ECS::Entity*>>& layers);

		/*============================================================================================
		* \brief		Entry point for sprite/shape collection under a single canvas entity.
		* \param[in]	engine        Reference to EnoEngine (provides entity manager + scene tree)
		* \param[in]	canvasEntity  The root canvas entity whose children are collected
		* \param[out]	layers        Map of canvas layer index → list of drawable entity pointers
		*//*========================================================================================*/
		static void collectCanvasChildren(
			Engine::EnoEngine& engine,
			ECS::Entity& canvasEntity,
			std::map<int, std::vector<ECS::Entity*>>& layers);

		/*============================================================================================
		* \brief		Recursively walk a scene node and collect all visible descendants that
		*				carry a TextComponent into the output list.
		* \param[in]	node          Root of the sub-tree to traverse
		* \param[in]	entityMng     Entity manager used to query components
		* \param[out]	textEntities  Accumulator list of text entity pointers
		*//*========================================================================================*/
		static void collectCanvasTextRecursive(
			Editor::Node* node,
			ECS::EntityManager& entityMng,
			std::vector<ECS::Entity*>& textEntities);

		/*============================================================================================
		* \brief		Entry point for text entity collection under a single canvas entity.
		* \param[in]	engine        Reference to EnoEngine (provides entity manager + scene tree)
		* \param[in]	canvasEntity  The root canvas entity whose text descendants are collected
		* \param[out]	textEntities  Accumulator list of text entity pointers
		*//*========================================================================================*/
		static void collectCanvasTextEntities(
			Engine::EnoEngine& engine,
			ECS::Entity& canvasEntity,
			std::vector<ECS::Entity*>& textEntities);

		/*============================================================================================
		* \brief		Draw all sprite/shape children of a canvas entity, sorted by layer.
		* \param[in]	engine          Reference to EnoEngine
		* \param[in]	cachedEntities  Pre-sorted drawable entity list (used by the outer loop)
		* \param[in]	canvasEntity    The canvas entity whose children are rendered
		* \param[in]	cameraTopLeft   Camera top-left offset applied to each entity's world pos
		*//*========================================================================================*/
		void renderCanvasEntities(
			Engine::EnoEngine& engine,
			std::vector<ECS::Entity>& cachedEntities,
			ECS::Entity& canvasEntity,
			const Math::eVec2<float>& cameraTopLeft);

		/*============================================================================================
		* \brief		Draw all text children of a canvas entity using drawText().
		* \param[in]	engine          Reference to EnoEngine
		* \param[in]	cachedEntities  Pre-sorted drawable entity list (used by the outer loop)
		* \param[in]	canvasEntity    The canvas entity whose text children are rendered
		*//*========================================================================================*/
		void renderCanvasText(
			Engine::EnoEngine& engine,
			std::vector<ECS::Entity>& cachedEntities,
			ECS::Entity& canvasEntity,
			const Math::eVec2<float>& cameraTopLeft);
	};

} // end of namespace UI
