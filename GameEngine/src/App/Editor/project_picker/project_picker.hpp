/*============================================================================================
* @file         project_picker.hpp
* @brief        Project picker dialog — open, create, and switch between projects.
*               Provides imGuiProjectPicker() which owns its own ImGui frame and renders
*               the Open/New-Project dialog before the main editor is displayed.
*//*========================================================================================*/

#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Engine { class EnoEngine; }

/*============================================================================================
 * \brief       Renders the project picker dialog (owns its own ImGui frame).
 *              Call instead of imGuiDisplay() when isProjectPickerOpen() is true.
 * \param[in]   engine_  Engine reference (needed for project switching)
*//*========================================================================================*/
void imGuiProjectPicker(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Renders the project picker as an inline modal popup.
 *              Call from inside an existing ImGui frame (e.g. imGuiDisplay).
 *              Opens the popup when isProjectPickerOpen() is true.
*//*========================================================================================*/
void imGuiProjectPickerPopup(Engine::EnoEngine& engine_);

/*============================================================================================
 * \brief       Returns true if the project picker dialog is currently open.
*//*========================================================================================*/
bool isProjectPickerOpen();

/*============================================================================================
 * \brief       Opens or closes the project picker dialog.
 *              Passing true also rescans available projects and pre-selects the current one.
*//*========================================================================================*/
void setProjectPickerOpen(bool open);
