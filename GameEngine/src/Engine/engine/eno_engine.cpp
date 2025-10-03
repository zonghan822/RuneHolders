#include "eno_engine.hpp"

namespace Engine
{
	void EnoEngine::setup()
	{
		window_.initialize("RuneHolders", configMng_.getWindowDim().first, configMng_.getWindowDim().second);

		// Systems init
		inputMng_.initialize();
		graphicsMng_.initialize(window_);
		audioMng_.init();

		setupResourceLoading();
	}

	// Thats why i added a callback version
	void EnoEngine::run(std::function<void()> gameUpdateCallback)
	{
		while (!window_.shouldClose())
		{
			updateTime(1.0);
			window_.pollEvents();
			
			// Handle window resize for graphics system
			auto fbSize= window_.getFramebufferSize();
			int width = fbSize.first, height = fbSize.second;
			if (width > 0 && height > 0) 
			{
				graphicsMng_.setProjectionMatrix(width, height);
			}

			// Begin graphics frame
			graphicsMng_.beginFrame(Graphics::Color::Black);  // Clear to black
			
			// Call game logic if provided
			if (gameUpdateCallback) {
				gameUpdateCallback();
			}

			audioMng_.process();

			// End graphics frame and present
			graphicsMng_.endFrame(window_);

			inputMng_.endFrame();
		}

	}

	void EnoEngine::shutDown() {

	}

	// Loader
	void EnoEngine::setupResourceLoading()
	{
		// Loading assets from directories
		loader_.loadSound(audioMng_.getSystem(), loader_.getMusicPath());
		loader_.loadSound(audioMng_.getSystem(), loader_.getSFXPath());

		// Loading shaders
		loader_.loadShaders(loader_.getShaderPath());

		// Loading images
		loader_.loadImages(loader_.getImagePath());

		// Loading fonts
		loader_.loadFonts(loader_.getFontPath(), 48);
	}

	void EnoEngine::updateTime(double fps_calc_interval) {
		// get elapsed time (in seconds) between previous and current frames
		static double prev_time = glfwGetTime();
		double curr_time = glfwGetTime();
		dt_ = curr_time - prev_time;
		prev_time = curr_time;

		// fps calculations
		static double count = 0.0; // number of game loop iterations
		static double start_time = glfwGetTime();
		// get elapsed time since very beginning (in seconds) ...
		double elapsed_time = curr_time - start_time;

		++count;

		// update fps at least every 10 seconds ...
		fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
		fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
		if (elapsed_time > fps_calc_interval) {
			fps_ = count / elapsed_time;
			start_time = curr_time;
			count = 0.0;
		}
	}
}