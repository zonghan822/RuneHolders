/*============================================================================================
* @file         eno_bridge_render.cpp
* @brief        Render and Text component bridge — size, text content, font size.
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/

#include "eno_bridge.hpp"
#include "../component/components/drawInfo.hpp"
#include "../component/components/text_component.hpp"
#include "../resources/eno_loader.hpp"

namespace Bridge
{
    static ECSComponents::RenderComponent* getRender(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::RenderComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::RenderComponent>(gEngine->getEntityManager());
    }

    static ECSComponents::TextComponent* getText(uint32_t sceneObjectID)
    {
        if (!gEngine) return nullptr;
        Editor::Node* node = gEngine->getEnoEditor().getSceneTree().getNodeBySceneObjectID(sceneObjectID);
        if (!node) return nullptr;
        if (!node->entity_.has<ECSComponents::TextComponent>(gEngine->getEntityManager())) return nullptr;
        return &node->entity_.get<ECSComponents::TextComponent>(gEngine->getEntityManager());
    }

    extern "C"
    {
        // ============= Render =============

        ENGINE_API float __stdcall GetRenderWidth(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->width : 0.0f;
        }

        ENGINE_API float __stdcall GetRenderHeight(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->height : 0.0f;
        }

        ENGINE_API void __stdcall SetRenderSize(uint32_t sceneObjectID, float w, float h)
        {
            if (auto* r = getRender(sceneObjectID))
            {
                r->width  = w;
                r->height = h;
            }
        }

        ENGINE_API void __stdcall SetTexturePath(uint32_t sceneObjectID, const char* textureName)
        {
            if (!textureName) return;
            auto* r = getRender(sceneObjectID);
            if (!r) return;
            auto tex = gEngine->getResourceLoader().getTexture(textureName);
            if (!tex) return;
            r->setTexture(tex, textureName);
        // ============= UV =============

        ENGINE_API float __stdcall GetUVStartX(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->uvStart.x : 0.0f;
        }

        ENGINE_API float __stdcall GetUVStartY(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->uvStart.y : 0.0f;
        }

        ENGINE_API float __stdcall GetUVEndX(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->uvEnd.x : 1.0f;
        }

        ENGINE_API float __stdcall GetUVEndY(uint32_t sceneObjectID)
        {
            auto* r = getRender(sceneObjectID);
            return r ? r->uvEnd.y : 1.0f;
        }

        ENGINE_API void __stdcall SetUV(uint32_t sceneObjectID, float startX, float startY, float endX, float endY)
        {
            if (auto* r = getRender(sceneObjectID))
            {
                r->uvStart = { startX, startY };
                r->uvEnd   = { endX,   endY   };
            }
        }

        // ============= Text =============

        ENGINE_API const char* __stdcall GetText(uint32_t sceneObjectID)
        {
            auto* t = getText(sceneObjectID);
            return t ? t->text : "";
        }

        ENGINE_API void __stdcall SetText(uint32_t sceneObjectID, const char* text)
        {
            if (auto* t = getText(sceneObjectID))
            {
                strncpy_s(t->text, text, sizeof(t->text) - 1);
                t->text[sizeof(t->text) - 1] = '\0';
            }
        }

        ENGINE_API float __stdcall GetFontSize(uint32_t sceneObjectID)
        {
            auto* t = getText(sceneObjectID);
            return t ? t->fontSize : 1.0f;
        }

        ENGINE_API void __stdcall SetFontSize(uint32_t sceneObjectID, float size)
        {
            if (auto* t = getText(sceneObjectID))
                t->fontSize = size;
        }
    }
}
