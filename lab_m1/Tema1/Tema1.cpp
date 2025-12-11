#include "lab_m1/Tema1/Tema1.h"
#include "components/transform.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace m1;

Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

void Tema1::Init()
{
    // Enable depth test for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set up the 3D camera
    auto camera = GetSceneCamera();
    camera->SetPosition(glm::vec3(0, 10, 15));  // Higher view to see the track
    camera->SetRotation(glm::vec3(-20, 0, 0));  // Look down slightly
    camera->Update();
    
    // Enable camera input so you can move around
    GetCameraInput()->SetActive(true);

    // Create a big green plane for the train terrain (100x100 units)
    {
        std::vector<VertexFormat> vertices = {
            VertexFormat(glm::vec3(-50, 0, -50), glm::vec3(0.2f, 0.8f, 0.2f), glm::vec3(0, 1, 0), glm::vec2(0, 0)),
            VertexFormat(glm::vec3( 50, 0, -50), glm::vec3(0.2f, 0.8f, 0.2f), glm::vec3(0, 1, 0), glm::vec2(1, 0)),
            VertexFormat(glm::vec3( 50, 0,  50), glm::vec3(0.2f, 0.8f, 0.2f), glm::vec3(0, 1, 0), glm::vec2(1, 1)),
            VertexFormat(glm::vec3(-50, 0,  50), glm::vec3(0.2f, 0.8f, 0.2f), glm::vec3(0, 1, 0), glm::vec2(0, 1))
        };

        std::vector<unsigned int> indices = {
            0, 1, 2,
            0, 2, 3
        };

        CreateMesh("plane", vertices, indices);
    }

    // Create geometric shapes for train components
    CreateBox("box", glm::vec3(0.8f, 0.4f, 0.2f));  // Orange-brown color for boxes
    CreateCylinder("cylinder", glm::vec3(0.3f, 0.3f, 0.3f));  // Dark gray for wheels/cylinders
    
    // Initialize the rail network
    InitializeRailNetwork();
    
    // Initialize train
    train.currentRail = railNetwork[0];  // Start at first rail
    train.progress = 0.0f;
    train.speed = 5.0f;  // Increased speed: 5 units per second (was 2.0)
    train.position = train.currentRail->startPosition;
    train.angle = CalculateTrainAngle(train.currentRail->endPosition - train.currentRail->startPosition);
    train.stopped = false;
    train.selectedDirection = -1;
}

void Tema1::FrameStart() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds) {
    // Update train movement
    UpdateTrainMovement(deltaTimeSeconds);
    
    // Render the big green plane (terrain for the train game)
    RenderMesh(meshes["plane"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));

    // Render rails
    RenderRails();

    // Render locomotive at train position (no wagon)
    RenderLocomotive(train.position, train.angle);
}

void Tema1::FrameEnd()
{
    DrawCoordinateSystem();
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}

void Tema1::OnKeyPress(int key, int mods)
{
    // Handle junction direction selection when train is stopped
    if (train.stopped) {
        HandleJunctionInput(key);
    }
}

void Tema1::OnKeyRelease(int key, int mods)
{
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema1::OnWindowResize(int width, int height)
{
}

void Tema1::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO = 0;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    glBindVertexArray(0);

    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema1::CreateBox(const char* name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices = {
        // Front face
        VertexFormat(glm::vec3(-0.5f, -0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(0, 0)),
        VertexFormat(glm::vec3( 0.5f, -0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        VertexFormat(glm::vec3( 0.5f,  0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(0, 1)),
        
        // Back face
        VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(0, 0)),
        VertexFormat(glm::vec3( 0.5f, -0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(1, 0)),
        VertexFormat(glm::vec3( 0.5f,  0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(0, 1)),
        
        // Right face
        VertexFormat(glm::vec3( 0.5f, -0.5f,  0.5f), color, glm::vec3(1, 0, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3( 0.5f, -0.5f, -0.5f), color, glm::vec3(1, 0, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3( 0.5f,  0.5f, -0.5f), color, glm::vec3(1, 0, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3( 0.5f,  0.5f,  0.5f), color, glm::vec3(1, 0, 0), glm::vec2(0, 1)),
        
        // Left face
        VertexFormat(glm::vec3(-0.5f, -0.5f,  0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(0, 1)),
        
        // Top face
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3( 0.5f,  0.5f,  0.5f), color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3( 0.5f,  0.5f, -0.5f), color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(0, 1, 0), glm::vec2(0, 1)),
        
        // Bottom face
        VertexFormat(glm::vec3(-0.5f, -0.5f,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3( 0.5f, -0.5f,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3( 0.5f, -0.5f, -0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), color, glm::vec3(0, -1, 0), glm::vec2(0, 1))
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  0, 2, 3,
        4, 6, 5,  4, 7, 6,
        8, 9, 10,  8, 10, 11,
        12, 14, 13,  12, 15, 14,
        16, 17, 18,  16, 18, 19,
        20, 22, 21,  20, 23, 22
    };

    CreateMesh(name, vertices, indices);
}

void Tema1::CreateCylinder(const char* name, glm::vec3 color, int segments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    
    float radius = 0.5f;
    float height = 1.0f;
    
    // Bottom circle center
    vertices.push_back(VertexFormat(glm::vec3(0, 0, -height/2), color, glm::vec3(0, 0, -1), glm::vec2(0.5f, 0.5f)));
    
    // Bottom circle vertices
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, -height/2), color, glm::vec3(0, 0, -1), glm::vec2(0, 0)));
    }
    
    // Top circle center
    int topCenterIdx = vertices.size();
    vertices.push_back(VertexFormat(glm::vec3(0, 0, height/2), color, glm::vec3(0, 0, 1), glm::vec2(0.5f, 0.5f)));
    
    // Top circle vertices
    int topStartIdx = vertices.size();
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, height/2), color, glm::vec3(0, 0, 1), glm::vec2(0, 0)));
    }
    
    // Bottom cap indices
    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    
    // Top cap indices
    for (int i = 0; i < segments; i++) {
        indices.push_back(topCenterIdx);
        indices.push_back(topStartIdx + i + 1);
        indices.push_back(topStartIdx + i);
    }
    
    // Side faces
    for (int i = 0; i < segments; i++) {
        int bottom1 = 1 + i;
        int bottom2 = 1 + i + 1;
        int top1 = topStartIdx + i;
        int top2 = topStartIdx + i + 1;
        
        indices.push_back(bottom1);
        indices.push_back(bottom2);
        indices.push_back(top1);
        
        indices.push_back(bottom2);
        indices.push_back(top2);
        indices.push_back(top1);
    }
    
    CreateMesh(name, vertices, indices);
}

void Tema1::RenderLocomotive(glm::vec3 position, float angle)
{
    // === LOCOMOTIVE STRUCTURE ===
    // Components positioned relative to a central base platform
    // Origin (0,0,0) is at the CENTER of each default shape
    
    glm::mat4 baseMatrix = glm::mat4(1);
    baseMatrix = glm::translate(baseMatrix, position);
    baseMatrix = glm::rotate(baseMatrix, angle, glm::vec3(0, 1, 0));
    
    // === DIMENSIONS ===
    const float WHEEL_RADIUS = 0.25f;
    const float WHEEL_WIDTH = 0.15f;
    const float PLATFORM_HEIGHT = 0.1f;
    const float PLATFORM_WIDTH = 1.2f;
    const float PLATFORM_LENGTH = 2.0f;
    const float CABIN_WIDTH = 1.0f;
    const float CABIN_HEIGHT = 1.0f;
    const float CABIN_LENGTH = 1.0f;
    const float MOTOR_RADIUS = 0.4f;
    const float MOTOR_LENGTH = 1.0f;
    
    // Calculate vertical positions
    const float PLATFORM_Y = WHEEL_RADIUS + PLATFORM_HEIGHT/2;  // Platform sits on top of wheels
    const float CABIN_Y = PLATFORM_Y + PLATFORM_HEIGHT/2 + CABIN_HEIGHT/2;  // Cabin sits on platform
    const float MOTOR_Y = PLATFORM_Y + PLATFORM_HEIGHT/2 + MOTOR_RADIUS;  // Motor sits on platform (half inside)
    
    // === 1. PLATFORM (base for everything) ===
    // Default box origin is at center
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, PLATFORM_Y, 0));  // Lift to wheel height
        modelMatrix = glm::scale(modelMatrix, glm::vec3(PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }
    
    // === 2. CABIN (vertical box at back) ===
    // Default box: center at (0,0,0), extends ±0.5 in each direction
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, CABIN_Y, -PLATFORM_LENGTH/2 + CABIN_LENGTH/2));  // Move to back
        modelMatrix = glm::scale(modelMatrix, glm::vec3(CABIN_WIDTH, CABIN_HEIGHT, CABIN_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }
    
    // === 3. MOTOR (horizontal cylinder at front) ===
    // Default cylinder: center at (0,0,0), axis along Z, extends ±0.5 in Z
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, MOTOR_Y, PLATFORM_LENGTH/2 - MOTOR_LENGTH/2));  // Move to front
        modelMatrix = glm::scale(modelMatrix, glm::vec3(MOTOR_RADIUS, MOTOR_RADIUS, MOTOR_LENGTH));
        RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
    }
    
    // === 4. WHEELS (6 wheels: 3 on each side) ===
    // Default cylinder: center at (0,0,0), axis along Z
    // Rotate 90° around Y to make axis along X (sideways)
    const float WHEEL_SPACING = PLATFORM_LENGTH / 4;  // Evenly spaced
    float wheelZPositions[] = {
        -WHEEL_SPACING,   // Back wheel
        0,                // Middle wheel
        WHEEL_SPACING     // Front wheel
    };
    
    for (int i = 0; i < 3; i++) {
        // Left wheel
        {
            glm::mat4 modelMatrix = baseMatrix;
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-PLATFORM_WIDTH/2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }
        
        // Right wheel (symmetric)
        {
            glm::mat4 modelMatrix = baseMatrix;
            modelMatrix = glm::translate(modelMatrix, glm::vec3(PLATFORM_WIDTH/2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }
    }
}

// === RAIL SYSTEM IMPLEMENTATION ===

void Tema1::InitializeRailNetwork()
{
    // Create a complex rail network with many junctions
    // Network layout (top view) - expanded with many more segments:
    //
    //                    [20]---[21]
    //                    /
    //        [16]---[17]---[18]---[19]
    //        /                    \
    //   [14]---[15]               [22]---[23]
    //   /
    // [0]---[1]---[2]---[3]---[4]---[5]
    //            \            /     \
    //             [6]---[7]---[8]   [9]---[10]
    //                  \            /
    //                   [11]---[12]---[13]
    //                        \
    //                         [24]---[25]
    
    // Main horizontal line (spine of the network)
    Rail* rail0 = new Rail(glm::vec3(-15, 0, 0), glm::vec3(-12, 0, 0));
    Rail* rail1 = new Rail(glm::vec3(-12, 0, 0), glm::vec3(-9, 0, 0));
    Rail* rail2 = new Rail(glm::vec3(-9, 0, 0), glm::vec3(-6, 0, 0));
    Rail* rail3 = new Rail(glm::vec3(-6, 0, 0), glm::vec3(-3, 0, 0));
    Rail* rail4 = new Rail(glm::vec3(-3, 0, 0), glm::vec3(0, 0, 0));
    Rail* rail5 = new Rail(glm::vec3(0, 0, 0), glm::vec3(3, 0, 0));
    
    // South branch from rail2
    Rail* rail6 = new Rail(glm::vec3(-9, 0, 0), glm::vec3(-7, 0, -4));
    Rail* rail7 = new Rail(glm::vec3(-7, 0, -4), glm::vec3(-4, 0, -4));
    Rail* rail8 = new Rail(glm::vec3(-4, 0, -4), glm::vec3(-1, 0, -2));
    
    // South extension
    Rail* rail9 = new Rail(glm::vec3(0, 0, 0), glm::vec3(2, 0, -3));
    Rail* rail10 = new Rail(glm::vec3(2, 0, -3), glm::vec3(4, 0, -6));
    
    // Deep south branch
    Rail* rail11 = new Rail(glm::vec3(-4, 0, -4), glm::vec3(-3, 0, -7));
    Rail* rail12 = new Rail(glm::vec3(-3, 0, -7), glm::vec3(0, 0, -8));
    Rail* rail13 = new Rail(glm::vec3(0, 0, -8), glm::vec3(3, 0, -6));
    
    // North branch from rail0
    Rail* rail14 = new Rail(glm::vec3(-15, 0, 0), glm::vec3(-14, 0, 3));
    Rail* rail15 = new Rail(glm::vec3(-14, 0, 3), glm::vec3(-11, 0, 4));
    
    // North extension
    Rail* rail16 = new Rail(glm::vec3(-11, 0, 4), glm::vec3(-8, 0, 5));
    Rail* rail17 = new Rail(glm::vec3(-8, 0, 5), glm::vec3(-5, 0, 6));
    Rail* rail18 = new Rail(glm::vec3(-5, 0, 6), glm::vec3(-2, 0, 6));
    Rail* rail19 = new Rail(glm::vec3(-2, 0, 6), glm::vec3(1, 0, 5));
    
    // Far north branch
    Rail* rail20 = new Rail(glm::vec3(-5, 0, 6), glm::vec3(-4, 0, 9));
    Rail* rail21 = new Rail(glm::vec3(-4, 0, 9), glm::vec3(-1, 0, 11));
    
    // East connections
    Rail* rail22 = new Rail(glm::vec3(1, 0, 5), glm::vec3(3, 0, 3));
    Rail* rail23 = new Rail(glm::vec3(3, 0, 3), glm::vec3(5, 0, 1));
    
    // South deep extension
    Rail* rail24 = new Rail(glm::vec3(0, 0, -8), glm::vec3(1, 0, -11));
    Rail* rail25 = new Rail(glm::vec3(1, 0, -11), glm::vec3(3, 0, -13));
    
    // Connect rails to build the complex network
    rail0->children.push_back(rail1);
    rail0->children.push_back(rail14);  // Junction: straight or north
    
    rail1->children.push_back(rail2);
    
    rail2->children.push_back(rail3);
    rail2->children.push_back(rail6);  // Junction: straight or south
    
    rail3->children.push_back(rail4);
    
    rail4->children.push_back(rail5);
    rail4->children.push_back(rail8);  // Junction: merge from south
    rail4->children.push_back(rail9);  // Junction: can go south-east
    
    rail5->children.push_back(rail0);  // Loop back to start
    rail5->children.push_back(rail23); // Or continue east
    
    // South branch connections
    rail6->children.push_back(rail7);
    
    rail7->children.push_back(rail8);
    rail7->children.push_back(rail11);  // Junction: continue or go deeper south
    
    rail8->children.push_back(rail4);  // Merge back to main line
    
    rail9->children.push_back(rail10);
    rail9->children.push_back(rail13);  // Junction: continue or merge from deep south
    
    rail10->children.push_back(rail13);
    
    // Deep south connections
    rail11->children.push_back(rail12);
    
    rail12->children.push_back(rail13);
    rail12->children.push_back(rail24);  // Junction: continue or go even deeper
    
    rail13->children.push_back(rail9);   // Loop back
    rail13->children.push_back(rail10);  // Junction: multiple options
    
    // North branch connections
    rail14->children.push_back(rail15);
    
    rail15->children.push_back(rail16);
    rail15->children.push_back(rail1);   // Junction: continue north or back to main
    
    rail16->children.push_back(rail17);
    
    rail17->children.push_back(rail18);
    rail17->children.push_back(rail20);  // Junction: straight or far north
    
    rail18->children.push_back(rail19);
    rail18->children.push_back(rail20);  // Junction: continue or branch
    
    rail19->children.push_back(rail22);
    rail19->children.push_back(rail5);   // Junction: east or back to main
    
    rail20->children.push_back(rail21);
    
    rail21->children.push_back(rail20);  // Can loop
    rail21->children.push_back(rail19);  // Or go back
    
    // East connections
    rail22->children.push_back(rail23);
    
    rail23->children.push_back(rail5);   // Back to main line
    rail23->children.push_back(rail10);  // Junction: or go south
    
    // Deep south extension
    rail24->children.push_back(rail25);
    
    rail25->children.push_back(rail24);  // Can turn around
    rail25->children.push_back(rail12);  // Or go back up
    
    // Store all rails in network
    railNetwork.push_back(rail0);
    railNetwork.push_back(rail1);
    railNetwork.push_back(rail2);
    railNetwork.push_back(rail3);
    railNetwork.push_back(rail4);
    railNetwork.push_back(rail5);
    railNetwork.push_back(rail6);
    railNetwork.push_back(rail7);
    railNetwork.push_back(rail8);
    railNetwork.push_back(rail9);
    railNetwork.push_back(rail10);
    railNetwork.push_back(rail11);
    railNetwork.push_back(rail12);
    railNetwork.push_back(rail13);
    railNetwork.push_back(rail14);
    railNetwork.push_back(rail15);
    railNetwork.push_back(rail16);
    railNetwork.push_back(rail17);
    railNetwork.push_back(rail18);
    railNetwork.push_back(rail19);
    railNetwork.push_back(rail20);
    railNetwork.push_back(rail21);
    railNetwork.push_back(rail22);
    railNetwork.push_back(rail23);
    railNetwork.push_back(rail24);
    railNetwork.push_back(rail25);
    
    std::cout << "Complex rail network initialized with " << railNetwork.size() << " segments" << std::endl;
    std::cout << "Controls: Arrow keys (LEFT/RIGHT/UP) to choose direction at junctions" << std::endl;
}

void Tema1::UpdateTrainMovement(float deltaTime)
{
    if (!train.currentRail) return;
    
    // If stopped at junction, wait for user input
    if (train.stopped) {
        return;  // Don't move until direction is selected
    }
    
    // Update progress along current rail
    float railLength = glm::length(train.currentRail->endPosition - train.currentRail->startPosition);
    float progressDelta = (train.speed * deltaTime) / railLength;
    train.progress += progressDelta;
    
    // Check if reached end of current rail
    if (train.progress >= 1.0f) {
        train.progress = 1.0f;  // Clamp to end
        
        // Check if this is a junction
        if (train.currentRail->isJunction()) {
            // Stop at junction and wait for input
            train.stopped = true;
            train.selectedDirection = -1;
            std::cout << "Train stopped at junction. Choose direction:" << std::endl;
            for (size_t i = 0; i < train.currentRail->children.size(); i++) {
                glm::vec3 dir = train.currentRail->children[i]->endPosition - 
                               train.currentRail->children[i]->startPosition;
                std::cout << "  [" << i << "] Direction: (" << dir.x << ", " << dir.z << ")" << std::endl;
            }
        } else {
            // Not a junction, move to next rail automatically
            Rail* nextRail = train.currentRail->getNext();
            if (nextRail) {
                train.currentRail = nextRail;
                train.progress = 0.0f;
            } else {
                // End of track - stop the train
                std::cout << "Train reached end of track!" << std::endl;
                train.stopped = true;
            }
        }
    }
    
    // Interpolate position along current rail (linear interpolation)
    glm::vec3 start = train.currentRail->startPosition;
    glm::vec3 end = train.currentRail->endPosition;
    train.position = glm::mix(start, end, train.progress);
    
    // Calculate train angle based on direction
    glm::vec3 direction = end - start;
    train.angle = CalculateTrainAngle(direction);
}

void Tema1::RenderRails()
{
    // Render all rail segments as lines/boxes
    for (Rail* rail : railNetwork) {
        glm::vec3 start = rail->startPosition;
        glm::vec3 end = rail->endPosition;
        glm::vec3 midpoint = (start + end) / 2.0f;
        
        // Calculate rail properties
        glm::vec3 direction = end - start;
        float length = glm::length(direction);
        float angle = atan2(direction.x, direction.z);
        
        // Render rail as a thin elongated box
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, midpoint + glm::vec3(0, 0.05f, 0));  // Slightly above ground
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.05f, length));
        
        // Color: red if it's a junction, gray otherwise
        glm::vec3 railColor = rail->isJunction() ? glm::vec3(1, 0, 0) : glm::vec3(0.5f, 0.5f, 0.5f);
        
        // Temporarily create a colored box for this rail
        // (In a real implementation, you'd want to cache these meshes)
        CreateBox("rail_temp", railColor);
        RenderMesh(meshes["rail_temp"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::HandleJunctionInput(int key)
{
    if (!train.stopped || !train.currentRail->isJunction()) return;
    
    int numChildren = train.currentRail->children.size();
    
    // Map arrow keys to directions
    int directionIndex = -1;
    
    if (key == GLFW_KEY_LEFT && numChildren > 0) {
        directionIndex = 0;  // First option (typically left)
    } else if (key == GLFW_KEY_UP && numChildren > 1) {
        directionIndex = 1;  // Second option (typically straight)
    } else if (key == GLFW_KEY_RIGHT && numChildren > 2) {
        directionIndex = 2;  // Third option (typically right)
    }
    
    // If valid direction selected, move to that rail
    if (directionIndex >= 0 && directionIndex < numChildren) {
        train.currentRail = train.currentRail->children[directionIndex];
        train.progress = 0.0f;
        train.stopped = false;
        train.selectedDirection = directionIndex;
        
        std::cout << "Direction " << directionIndex << " selected. Train moving..." << std::endl;
    }
}

float Tema1::CalculateTrainAngle(glm::vec3 direction)
{
    // Calculate angle in radians from direction vector
    // atan2 gives angle from positive X axis, we want angle from positive Z axis
    return atan2(direction.x, direction.z);
}
