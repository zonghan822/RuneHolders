#include "glapp.h"
#include "GSM.hpp"
#include <iostream>

static void draw();
static void update();
static void init();


int main() {
	init();

	GSM_Initialize(GS_LEVEL1);


	while (current != GS_QUIT)
	{
		if (current != GS_RESTART) {
			GSM_Update();
			fpLoad();
		}
		else {
			next = previous;
			current = previous;
		}

		fpInitialize();

		while (next == current) {
			//AESysFrameStart();
			fpUpdate();
			fpDraw();

			if (glfwWindowShouldClose(GLApp::ptr_window)) {
				next = GS_QUIT;
			}
			update();
			draw();

			//AESysFrameEnd();
		}

		fpFree();
		if (next != GS_RESTART) {
			fpUnload();
		}

		previous = current;
		current = next;
	}

	GLApp::cleanup();
}



static void init() {
	// start with a 16:9 aspect ratio
	if (!GLApp::init(2400, 1350, "Tutorial 0 | Setting up OpenGL 4.5")) {
		std::cout << "Unable to create OpenGL context" << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

static void draw() {
	// render scene
	GLApp::draw();

	// swap buffers: front <-> back
	// GLApp::ptr_window is handle to window that defines the OpenGL context
	glfwSwapBuffers(GLApp::ptr_window);
}

static void update() {
	// process events if any associated with input devices
	glfwPollEvents();

	// main loop computes fps and other time related stuff once for all apps ...
	GLApp::update_time(1.0);

	// animate scene
	GLApp::update();
}