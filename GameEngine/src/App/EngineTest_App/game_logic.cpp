#include "game_logic.hpp"
#include "math/math.hpp"
#include "../Engine/collision/collision.hpp"



//for testing purposes
#include <random>
static std::mt19937 rng(std::random_device{}());
// returns a random float between min and max
float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

//throw std::runtime_error("error logging");

void GameApplication::initialize() {
    // Setup engine systems first
    engine_.setup();

    // Setup all game content
    setupGameEntities();
    setupGameAudio();
    setupGameEvents();
    setupGameMath();
    setupImGui();

    std::cout << "Game initialized successfully!" << std::endl;
}

void GameApplication::run() {
    std::cout << "Starting game..." << std::endl;

    // Start the engine with game update callback
    engine_.run([this]() {
        updateGameLogic();  // This gets called every frame
        });
    
    //------------------------------------HARDCODED TERMINATE OF IMGUI FUNCTIONS------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //------------------------------------END OF HARDCODED TERMINATE OF IMGUI FUNCTIONS-----------------------------

}

void GameApplication::setupGameEntities() {
    // To setup the entities in our game
}

void GameApplication::setupGameAudio() {
    // Setup audio
}

void GameApplication::setupGameEvents() {
    // setup event bus
}

void GameApplication::setupGameMath() {
    // math lib and maybe collision
    std::cout << "Calling math function\n";
    Math::eVec2 vec{ 1.1f, 1.2f };
    Math::eVec2 vec2{ 1.3f, 2.f };
    vec += vec2;
    std::cout << "vector : " << vec.v1 << ", " << vec.v2 << std::endl;

    Collision::RunBasicCollisionTest();

    //initialize collision system
    collisionSystem_ = std::make_unique<Collision::CollisionSystem>(engine_.getEventBus());
}

void GameApplication::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    GLFWwindow* window = engine_.getWindow().getWindow();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); 
}

void GameApplication::demonstrateSignals() {
    // signalling code
}

void GameApplication::updateGameLogic() {
    // This is called every frame (replaces your while loop in eno_engine.cpp run())
    updateEntities();
    updateAudio();
    updateImGui();
    // Add any other per-frame game logic here
    drawShapes();
    
}

void GameApplication::updateEntities() {
    // Update game entities each frame
    // Add entity movement, AI, collision detection etc here

    // update collision system every frame
    if (collisionSystem_) {
        collisionSystem_->Update(engine_.getEntityManager());
    }
}

void GameApplication::updateAudio() {
    // audio update
}

void GameApplication::updateImGui() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // (kim) get delta time for smooth movement
    ImGuiIO& io = ImGui::GetIO();
    float deltaTime = io.DeltaTime;

    // (kim) WASD movement for indicator (always active, no need to focus on ImGui)
    GLFWwindow* window = engine_.getWindow().getWindow();
    float moveDelta = moveSpeed_ * deltaTime;

    /* (kim added WASD Keys!) */
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) indicatorY_ -= moveDelta;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) indicatorY_ += moveDelta;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) indicatorX_ -= moveDelta;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) indicatorX_ += moveDelta;
   
    //Option Menu
    const char* shapes[] = {"Circle" , "Rectangle"};
    const char* colors[] = {"Green"  ,     "Red"  };
    const char* colliderShapes[] = { "None", "Circle", "Rectangle" };

    //Values to set for entities
    //static float           eCoordX          = 600;
    //static float           eCoordY          = 425;
    static float           eWidth           = 10;
    static float           eHeight          = 10;
    static int             eColor           = Color::Green;
    //static int             eShapeIndex      = Shapes::Rectangle;
    static int             eShapeIndex      = 0;         // 0 = circle, 1 = Rectangle
    static int             selectedCollider = 0;         // for the newly spawned entity
    static float           colliderWidth    = 10.0f;
    static float           colliderHeight   = 10.0f;
    static float           colliderRadius    = 5.0f;

    ImGui::NewFrame();
    ImGui::Begin("IMGUI CHECKLIST");

    //FPS and Time Staggering for Displaying FPS
    static float shown_fps = 0.0f;
    static double next_update = 0.0;         // next time we refresh the text
    double now = ImGui::GetTime();           // imgui monotonic time (seconds)

    if (now >= next_update) {
        shown_fps = io.Framerate;            // sample current FPS
        next_update = now + 0.5;             // update every 0.5s (tune this)
    }
    ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
    ImGui::Text("Frame (%.1f FPS)", shown_fps);

    ImGui::Separator();
    ImGui::Text("Indicator Controls (WASD)");
    ImGui::Text("Position (%.1f, %.1f)", indicatorX_, indicatorY_);
    ImGui::DragFloat("Move Speeed", &moveSpeed_, 10.0f, 50.0f, 500.0f);

    ImGui::Separator();

    //Changing Entity Values
    
    //ImGui::DragFloat("Size Width", &scalex);
    //ImGui::DragFloat("Size Height", &scaley);

    //Button Options
    //ImGui::DragFloat("New Entity X Pos",  &eCoordX);
    //ImGui::DragFloat("New Entity Y Pos",  &eCoordY);

    // entity spawn settings
    ImGui::Text("Entity Spawn Settings");
    ImGui::DragFloat("Entity Width",  &eWidth, 0.5f, 1.0f, 100.0f);
    ImGui::DragFloat("Entity Height", &eHeight, 0.5f, 1.0f, 100.0f);
    //dropdoawn menu
    ImGui::Combo("Shape", &eShapeIndex, shapes, IM_ARRAYSIZE(shapes));
    ImGui::Combo("Color", &eColor,      colors, IM_ARRAYSIZE(colors));

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

    ImGui::Separator();
    
    auto& graphics = engine_.getGraphicsManager();
    //Red indicator rectangle that shows where the entity will be spawned.  
    graphics.drawRect(
        Math::eVec2<float>{indicatorX_, indicatorY_},  // Position
        Math::eVec2<float>{10, 10},            // Size 10 wide 10 tall
        Graphics::Color::Red,                 //  Colour
        true                                  //  True
    );

    //Spawn Button (with optional collider)
    if (ImGui::Button("Spawn Entity")) {

        //e.set(ECSComponents::DrawInfoComponent("Square"));
        //e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent{ "NAME" });

        //SAMPLE DRAWINFOCOMPONENT LOOKS LIKE THIS: DrawInfoComponent(float xPosition, float yPosition, float w, float h, const char* n, int c)
        ECSComponents::DrawInfoComponent DIC { indicatorX_, indicatorY_, eWidth, eHeight, shapes[eShapeIndex], eColor};
        ECS::Entity e =  engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);

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
            if(selectedCollider == 1) {//circle
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

    //Delete Button?




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

        std::cout << "Cloned "<< counter <<  " Entity\n";
    }

    ImGui::Separator();

    //collision visualization toggle
    ImGui::Checkbox("Show Colliders", &showColliders_);

    ImGui::End();
}


void GameApplication::drawShapes() { // For testing
    auto& graphics = engine_.getGraphicsManager();

    auto& entityManager = engine_.getEntityManager();

    using drawInfo = ECSComponents::DrawInfoComponent;

    //thinking here i do for each drawable entity: draw.
    // draw all entities
    std::vector<ECS::Entity> EntityList = engine_.getEntityManager().findEntity<ECSComponents::DrawInfoComponent>();

    for (ECS::Entity& entity : EntityList) {
        if (entity.has<ECSComponents::DrawInfoComponent>()) {
            graphics.drawRect(
                Math::eVec2<float>{entity.get<drawInfo>().xPos, entity.get<drawInfo>().yPos},
                Math::eVec2<float>{entity.get<drawInfo>().width, entity.get<drawInfo>().height},
                entity.get<drawInfo>().color,
                true
            );
        }
    }

    // (kim) visualize colliders if enabled
    if (showColliders_ && collisionSystem_) {
        collisionSystem_->VisualizeColliders(graphics, entityManager);
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
