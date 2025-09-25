#include "game_logic.hpp"
#include "math/math.hpp"


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
    static float value = 0.0f;
    ImGui::DragFloat("Size Width", &scalex);
    ImGui::DragFloat("Size Height", &scaley);

    //Spawn Button
    if (ImGui::Button("Spawn Entity")) {

        //ECS::Entity e =  engine_.getEntityManager().create<Component Type goes in here>(value for comp type);
        ECS::Entity e =  engine_.getEntityManager().create<ECSComponents::NameComponent>("Animal");
        //e.add(ECSComponents::NameComponent("Human"));
        std::cout << "Spawning New Entity\n";
        auto name = e.get<ECSComponents::NameComponent>().name;
        std::cout << name << '\n';
        
        
    }
    //Clone Button
    if (ImGui::Button("Clone Entity")) {
        std::cout << "Clone Last Entity\n";
    }
    ImGui::End();
}


void GameApplication::drawShapes() { // For testing
    auto& graphics = engine_.getGraphicsManager();


    // Draw red rect
    graphics.drawRect(
        Math::eVec2<float>{100.0f, 100.0f},  // Position
        Math::eVec2<float>{scalex, scaley},  // Size 200 wide 150 tall
        Graphics::Color::Red,                 // Colour
        true                                 // Filled
    );

    // Draw green circ
    graphics.drawCircle(
        Math::eVec2<float>{500.0f, 200.0f},  // Center
        75.0f,                               // Radius
        Graphics::Color::Green,              // Colour
        true,                                // Filled
        32                                   // Segments
    );

    // Draw blue tri
    graphics.drawTriangle(
        Math::eVec2<float>{300.0f, 400.0f},  // Bot left
        Math::eVec2<float>{450.0f, 300.0f},  // Top
        Math::eVec2<float>{600.0f, 400.0f},  // Bot right
        Graphics::Color::Blue,               // Colour
        true                                 // Filled
    );


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
