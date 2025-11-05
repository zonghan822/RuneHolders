#pragma once

// ECS Components
#include "../component/components/drawInfo.hpp"
#include "../component/components/transform.hpp"
#include "../component/components/visibility.hpp"

// IMGUI
#include "imgui.h"

// For editor state
#include "../../src/App/Editor/game_logic.hpp"		

// Base class template for in game buttons
class Button 
{
public:
	virtual void run() = 0;


	// Accessors
	void getDrawInfo();
	void getTransform();
	void getVisible();

	// Mutators
	void setDrawInfo();
	void setTransform();
	void setVisible();

private:
	// ECS components
	ECSComponents::DrawInfoComponent drawInfo;
	ECSComponents::TransformComponent transform;
	ECSComponents::VisibilityComponent visible;

};

// Exit button changes playing editor state to stopped
class exitButton : public Button 
{
public:
	void run() override;
};