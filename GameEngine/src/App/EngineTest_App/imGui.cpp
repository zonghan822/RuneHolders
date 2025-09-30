#include "imGui.hpp"
#include "game_logic.hpp"

extern GameApplication game;

//collision system
std::unique_ptr<Collision::CollisionSystem> collisionSystem_;
bool showColliders_ = true;
std::unordered_map<std::string, double> systemTimes_; //actual system time
std::unordered_map<std::string, double> throttledTimes_; //used for displaying throttled timing(more user friendly)



float indicatorX_ = 600.0f;
float indicatorY_ = 425.0f;
float moveSpeed_ = 200.0f; // pixels per second

float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

void imGuiInit(GLFWwindow* window){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}


void imGuiDisplay(GLFWwindow* window,Engine::EnoEngine& engine_) {

	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();



	// (kim) get delta time for smooth movement
	ImGuiIO& io = ImGui::GetIO();
	float deltaTime = io.DeltaTime;

	// (kim) WASD movement for indicator (always active, no need to focus on ImGui)
	float moveDelta = moveSpeed_ * deltaTime;

	/* (kim added WASD Keys!) */
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) indicatorY_ -= moveDelta;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) indicatorY_ += moveDelta;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) indicatorX_ -= moveDelta;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) indicatorX_ += moveDelta;

	//Option Menu
	const char* shapes[] = { "Circle" , "Rectangle" };
	const char* colors[] = { "Green"  ,     "Red" };
	const char* colliderShapes[] = { "None", "Circle", "Rectangle" };

	//Values to set for entities
	//static float           eCoordX          = 600;
	//static float           eCoordY          = 425;
	static float           eWidth = 10;
	static float           eHeight = 10;
	static int             eColor = Color::Green;
	static int             eShapeIndex = Shapes::Rectangle;
	//static int             eShapeIndex      = 0;         // 0 = circle, 1 = Rectangle
	static int             selectedCollider = 0;         // for the newly spawned entity
	static float           colliderWidth = 10.0f;
	static float           colliderHeight = 10.0f;
	static float           colliderRadius = 5.0f;




	static float shown_fps = 0.0f;
	static double next_update = 0.0;         // next time we refresh the text
	double now = ImGui::GetTime();           // imgui monotonic time (seconds)

	if (now >= next_update) {
		shown_fps = io.Framerate;            // sample current FPS
		throttledTimes_ = systemTimes_;
		next_update = now + 0.5;             // update every 0.5s (tune this)
	}








	if (ImGui::Button("Main"))     activeWindow = WindowID::Main;
	if (ImGui::Button("Edit"))     activeWindow = WindowID::Edit;
	if (ImGui::Button("FPS"))      activeWindow = WindowID::FPS;
	//FPS and Time Staggering for Displaying FPS

	if (activeWindow == WindowID::Main) {
		ImGui::Begin("Main");

		ImGui::Separator();
		ImGui::Text("Indicator Controls (WASD)");
		ImGui::Text("Position (%.1f, %.1f)", indicatorX_, indicatorY_);
		ImGui::DragFloat("Move Speeed", &moveSpeed_, 10.0f, 50.0f, 500.0f);
		//collision visualization toggle
		ImGui::Checkbox("Show Colliders", &showColliders_);

		// entity spawn settings
		ImGui::Text("Entity Spawn Settings");
		ImGui::DragFloat("Entity Width", &eWidth, 0.5f, 1.0f, 100.0f);
		ImGui::DragFloat("Entity Height", &eHeight, 0.5f, 1.0f, 100.0f);
		//dropdoawn menu
		ImGui::Combo("Shape", &eShapeIndex, shapes, IM_ARRAYSIZE(shapes));
		ImGui::Combo("Color", &eColor, colors, IM_ARRAYSIZE(colors));

		ImGui::Separator();

		//collider settings
		ImGui::Text("Collider Settings");
		ImGui::Combo("ColliderType", &selectedCollider, colliderShapes, IM_ARRAYSIZE(colliderShapes));

		if (selectedCollider == 1) { // circle
			ImGui::DragFloat("Collider Width", &colliderWidth, 0.5f, 1.0f, 100.0f);
		}
		else if (selectedCollider == 2) { // rectangle
			ImGui::DragFloat("Collider Width", &colliderWidth, 0.5f, 1.0f, 100.0f);
			ImGui::DragFloat("Collider Height", &colliderHeight, 0.5f, 1.0f, 100.0f);
		}


		//Spawn Button (with optional collider)
		if (ImGui::Button("Spawn Entity")) {
			//e.set(ECSComponents::DrawInfoComponent("Square"));
			//e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent{ "NAME" });
			//SAMPLE DRAWINFOCOMPONENT LOOKS LIKE THIS: DrawInfoComponent(float xPosition, float yPosition, float w, float h, const char* n, int c)
			ECSComponents::DrawInfoComponent DIC{ indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor };
			ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);

			//adding name to the entity only for destroy() debug purposes
			e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent("John"));
			if (e.get<ECSComponents::NameComponent>().name == ECSComponents::NameComponent("John")) std::cout << "he john\n";


			// add collider if selected
			if (selectedCollider != 0) {
				// first add transform component (required for collision)
				ECSComponents::TransformComponent transform{
					{indicatorX_, indicatorY_, 0.0f},
					{0.0f, 0.0f, 0.0f},
					{1.0f, 1.0f, 1.0f}
				};

				e.add<ECSComponents::TransformComponent>(transform);

				// then add appropriate collider
				if (selectedCollider == 1) {//circle
					e.add<Collision::CollisionComponent>(Collision::CollisionComponent(colliderRadius));

				}
				else if (selectedCollider == 2) { // rectangle
					e.add<Collision::CollisionComponent>(Collision::CollisionComponent(colliderWidth, colliderHeight)
					);
				}

				std::cout << "Spawned entity WITH collider\n";
			}
			else {
				std::cout << "Spawned entity without collider\n";
			}
		}



		//Clone Button
		if (ImGui::Button("Clone 50 Entities")) {
			static size_t counter = 0;
			for (size_t i = 0; i < 50; i++) {
				float randX = randomFloat(0.0f, 1200.0f); //screen width
				float randY = randomFloat(0.0f, 900.0f); // screen height
				//HARDCODED(COPIED) SPAWN CODE INTO HERE
				ECSComponents::DrawInfoComponent DIC{ randX, randY, eWidth, eHeight, shapes[eShapeIndex], eColor };
				ECS::Entity e = engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);
			}
			counter += 50;

			std::cout << "Cloned " << counter << " Entity\n";
		}

		ImGui::End();
	}


	if (activeWindow == WindowID::Edit) {
		ImGui::Begin("Edit");
		const char* filters[] = { "Name", "Type" , "ID" };
		static int FilterType = Filters::Name;
		std::vector<ECS::Entity> toDelete;

		//THIS SHIT NOT WORKING!!!!!!
		ImGui::Combo("Filters", &FilterType, filters, IM_ARRAYSIZE(filters));
		if (FilterType == Filters::Name) {

			if (ImGui::Button("Delete John")) {
				for (ECS::Entity& e : engine_.getEntityManager().findEntitybyType<ECSComponents::NameComponent>()) {
					if (e.get<ECSComponents::NameComponent>().name == ECSComponents::NameComponent("John")) {
						toDelete.push_back(e);
					}
				}
			}
		}

		if (FilterType == Filters::Type) {
			//do another list to look for the type here
			for (ECS::Entity& e : engine_.getEntityManager().findEntitybyType<>()) {

			}

		}
		if (FilterType == Filters::ID) {

			for (ECS::Entity& e : engine_.getEntityManager().findEntitybyType<>()) {

			}

		}

		for (auto& a : toDelete) {
			a.destroy();
		}

		ImGui::End();
	}


	if (activeWindow == WindowID::FPS) {
		ImGui::Begin("FPS");
		ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
		ImGui::Text("Frame (%.1f FPS)", shown_fps);
		ImGui::Text("Per-System Timings:");
		for (auto& time : throttledTimes_) {
			ImGui::Text("%s: %.3f ms", time.first.c_str(), time.second / 1000.0);
			// divide by 1000 to show microseconds as milliseconds
		}


		ImGui::End();
	}
}






void imGuiTerminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}