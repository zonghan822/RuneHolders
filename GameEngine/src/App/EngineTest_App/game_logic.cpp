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
}

void GameApplication::updateAudio() {
    // audio update
}

void GameApplication::updateImGui() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    //Option Menu
    const char* shapes[] = {"Circle" , "Rectangle"};
    const char* colors[] = {"Green"  ,     "Red"  };

    //Values to set for entities
    static float           eCoordX          = 600;
    static float           eCoordY          = 425;
    static float           eWidth           = 10;
    static float           eHeight          = 10;
    static int             eColor           = Color::Green;
    static int             eShapeIndex      = Shapes::Rectangle;


    ImGui::NewFrame();
    ImGui::Begin("IMGUI CHECKLIST");

    //FPS and Time Staggering for Displaying FPS
    static float shown_fps = 0.0f;
    static double next_update = 0.0;         // next time we refresh the text
    double now = ImGui::GetTime();           // imgui monotonic time (seconds)

    ImGuiIO& io = ImGui::GetIO();
    if (now >= next_update) {
        shown_fps = io.Framerate;            // sample current FPS
        next_update = now + 0.5;             // update every 0.5s (tune this)
    }
    ImGui::Text("Application average %.3f ms", 1000.0f / shown_fps);
    ImGui::Text("Frame (%.1f FPS)", shown_fps);

    //Changing Entity Values
    ImGui::DragFloat("Size Width", &scalex);
    ImGui::DragFloat("Size Height", &scaley);

    //Button Options
    ImGui::DragFloat("New Entity X Pos",  &eCoordX);
    ImGui::DragFloat("New Entity Y Pos",  &eCoordY);
    ImGui::DragFloat("New Entity Width",  &eWidth);
    ImGui::DragFloat("New Entity Height", &eHeight);
    

    //Dropdown Menu
    ImGui::Combo("Shape: ", &eShapeIndex, shapes, IM_ARRAYSIZE(shapes));
    ImGui::Combo("Color: ", &eColor,      colors, IM_ARRAYSIZE(colors));




    auto& graphics = engine_.getGraphicsManager();
    //Red indicator rectangle that shows where the entity will be spawned.  
    graphics.drawRect(
        Math::eVec2<float>{eCoordX, eCoordY},  // Position
        Math::eVec2<float>{10, 10},            // Size 10 wide 10 tall
        Graphics::Color::Red,                 //  Colour
        true                                  //  True
    );

    //Spawn Button
    if (ImGui::Button("Spawn Entity")) {

        //e.set(ECSComponents::DrawInfoComponent("Square"));
        //e.add<ECSComponents::NameComponent>(ECSComponents::NameComponent{ "NAME" });

        //SAMPLE DRAWINFOCOMPONENT LOOKS LIKE THIS: DrawInfoComponent(float xPosition, float yPosition, float w, float h, const char* n, int c)
        ECSComponents::DrawInfoComponent DIC {eCoordX, eCoordY, eWidth, eHeight, shapes[eShapeIndex], eColor};
        ECS::Entity e =  engine_.getEntityManager().create<ECSComponents::DrawInfoComponent>(DIC);

        std::cout << e.get<ECSComponents::DrawInfoComponent>();
        std::cout << "Spawning New Entity\n";
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
    ImGui::End();
}


void GameApplication::drawShapes() { // For testing
    auto& graphics = engine_.getGraphicsManager();

    auto& entityManager = engine_.getEntityManager();

    using drawInfo = ECSComponents::DrawInfoComponent;

    //thinking here i do for each drawable entity: draw.
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



    //// Draw red rect
    //graphics.drawRect(
    //    Math::eVec2<float>{100.0f, 100.0f},  // Position
    //    Math::eVec2<float>{scalex, scaley},  // Size 200 wide 150 tall
    //    Graphics::Color::Red,                 // Colour
    //    true                                 // Filled
    //);




    // Draw green circ
    //graphics.drawCircle(
    //    Math::eVec2<float>{500.0f, 200.0f},  // Center
    //    75.0f,                               // Radius
    //    Graphics::Color::Green,              // Colour
    //    true,                                // Filled
    //    32                                   // Segments
    //);

    //// Draw blue tri
    //graphics.drawTriangle(
    //    Math::eVec2<float>{300.0f, 400.0f},  // Bot left
    //    Math::eVec2<float>{450.0f, 300.0f},  // Top
    //    Math::eVec2<float>{600.0f, 400.0f},  // Bot right
    //    Graphics::Color::Blue,               // Colour
    //    true                                 // Filled
    //);


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
