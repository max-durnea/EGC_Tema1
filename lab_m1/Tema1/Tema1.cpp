#include "lab_m1/Tema1/Tema1.h"
#include "components/transform.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cfloat>

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
    camera->SetPosition(glm::vec3(0, 16, 26));  // Higher view to see the enlarged track
    camera->SetRotation(glm::vec3(-20, 0, 0));  // Look down slightly
    camera->Update();

    // Enable camera input so you can move around
    GetCameraInput()->SetActive(true);

    // Create geometric shapes for train components
    CreateBox("box", glm::vec3(0.8f, 0.4f, 0.2f));  // Orange-brown color for boxes
    CreateCylinder("cylinder", glm::vec3(0.3f, 0.3f, 0.3f));  // Dark gray for wheels/cylinders

    // Create station shapes (3 different stations)
    CreateBox("station_cube", glm::vec3(0.9f, 0.9f, 0.1f));  // Yellow-ish cube station
    CreateCylinder("station_cylinder", glm::vec3(0.8f, 0.1f, 0.1f));  // Red-ish cylinder station
    CreatePyramid("station_pyramid", glm::vec3(0.1f, 0.8f, 0.8f));  // Cyan-ish pyramid station

    // Create rail meshes (different colors for different types)
    // Normal rail - black
    CreateBox("rail_normal", glm::vec3(0.1f, 0.1f, 0.1f));

    // Bridge rail colors - 4 alternating colors (longitudinal stripes)
    CreateBox("rail_bridge_stripe1", glm::vec3(1.0f, 0.6f, 0.2f));  // Orange
    CreateBox("rail_bridge_stripe2", glm::vec3(0.9f, 0.9f, 0.9f));  // White
    CreateBox("rail_bridge_stripe3", glm::vec3(1.0f, 0.6f, 0.2f));  // Orange
    CreateBox("rail_bridge_stripe4", glm::vec3(0.9f, 0.9f, 0.9f));  // White

    // Tunnel rail colors - 4 alternating colors (transversal stripes)
    CreateBox("rail_tunnel_stripe1", glm::vec3(0.5f, 0.3f, 0.1f));  // Dark brown
    CreateBox("rail_tunnel_stripe2", glm::vec3(0.7f, 0.7f, 0.7f));  // Light gray
    CreateBox("rail_tunnel_stripe3", glm::vec3(0.5f, 0.3f, 0.1f));  // Dark brown
    CreateBox("rail_tunnel_stripe4", glm::vec3(0.7f, 0.7f, 0.7f));  // Light gray

    // NO pre-rendered terrain - terrain will be rendered dynamically under each rail

    // Initialize the rail network
    InitializeRailNetwork();
}

void Tema1::FrameStart() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds) {
    // Render full terrain FIRST (as background)
    RenderFullTerrain();

    // Render all rails on top of terrain
    RenderRails();

    // Update train movement
    UpdateTrainMovement(deltaTimeSeconds);

    // Render the locomotive at its current position
    RenderLocomotive(train.position, train.angle);
}

void Tema1::FrameEnd()
{
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}

void Tema1::OnKeyPress(int key, int mods)
{
    // Handle junction direction selection with WASD
    if (train.stopped && train.currentRail && train.currentRail->isJunction()) {
        if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_D) {
            HandleJunctionInput(key);
            return;
        }
    }

    // Handle SPACE to continue on default path at junction
    if (key == GLFW_KEY_SPACE && train.stopped && train.currentRail && train.currentRail->isJunction()) {
        // Take the first (default) path
        if (!train.currentRail->children.empty()) {
            train.currentRail = train.currentRail->children[0];
            train.progress = 0.0f;
            train.stopped = false;
            std::cout << "Continuing on default path" << std::endl;
        }
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
        VertexFormat(glm::vec3(0.5f, -0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f,  0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(0, 0, 1), glm::vec2(0, 1)),

        // Back face
        VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(0.5f, -0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f,  0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(0, 0, -1), glm::vec2(0, 1)),

        // Right face
        VertexFormat(glm::vec3(0.5f, -0.5f,  0.5f), color, glm::vec3(1, 0, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(0.5f, -0.5f, -0.5f), color, glm::vec3(1, 0, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f,  0.5f, -0.5f), color, glm::vec3(1, 0, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(0.5f,  0.5f,  0.5f), color, glm::vec3(1, 0, 0), glm::vec2(0, 1)),

        // Left face
        VertexFormat(glm::vec3(-0.5f, -0.5f,  0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(-1, 0, 0), glm::vec2(0, 1)),

        // Top face
        VertexFormat(glm::vec3(-0.5f,  0.5f,  0.5f), color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(0.5f,  0.5f,  0.5f), color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f,  0.5f, -0.5f), color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f,  0.5f, -0.5f), color, glm::vec3(0, 1, 0), glm::vec2(0, 1)),

        // Bottom face
        VertexFormat(glm::vec3(-0.5f, -0.5f,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(0.5f, -0.5f,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f, -0.5f, -0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 1)),
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
    vertices.push_back(VertexFormat(glm::vec3(0, 0, -height / 2), color, glm::vec3(0, 0, -1), glm::vec2(0.5f, 0.5f)));

    // Bottom circle vertices
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, -height / 2), color, glm::vec3(0, 0, -1), glm::vec2(0, 0)));
    }

    // Top circle center
    int topCenterIdx = vertices.size();
    vertices.push_back(VertexFormat(glm::vec3(0, 0, height / 2), color, glm::vec3(0, 0, 1), glm::vec2(0.5f, 0.5f)));

    // Top circle vertices
    int topStartIdx = vertices.size();
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, height / 2), color, glm::vec3(0, 0, 1), glm::vec2(0, 0)));
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
void Tema1::CreateTerrainQuad(const char* name, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 color)
{
    std::vector<VertexFormat> vertices = {
        VertexFormat(p1, color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        VertexFormat(p2, color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        VertexFormat(p3, color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        VertexFormat(p4, color, glm::vec3(0, 1, 0), glm::vec2(0, 1)),
    };
    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };
    CreateMesh(name, vertices, indices);
}
void Tema1::CreatePyramid(const char* name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices = {
        // Base (square)
        VertexFormat(glm::vec3(-0.5f, 0, -0.5f), color, glm::vec3(0, -1, 0), glm::vec2(0, 0)),
        VertexFormat(glm::vec3(0.5f, 0, -0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        VertexFormat(glm::vec3(0.5f, 0,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(1, 1)),
        VertexFormat(glm::vec3(-0.5f, 0,  0.5f), color, glm::vec3(0, -1, 0), glm::vec2(0, 1)),

        // Apex
        VertexFormat(glm::vec3(0, 1, 0), color, glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f)),
    };

    std::vector<unsigned int> indices = {
        // Base
        0, 1, 2,
        0, 2, 3,

        // Side faces
        0, 1, 4,  // Front face
        1, 2, 4,  // Right face
        2, 3, 4,  // Back face
        3, 0, 4   // Left face
    };

    CreateMesh(name, vertices, indices);
}

void Tema1::RenderLocomotive(glm::vec3 position, float angle)
{
    // === LOCOMOTIVE STRUCTURE ===
    // Components positioned relative to a central base platform
    // Origin (0,0,0) is at the CENTER of each default shape

    const float TRAIN_SCALE = 0.6f;
    glm::mat4 baseMatrix = glm::mat4(1);
    baseMatrix = glm::translate(baseMatrix, position);
    baseMatrix = glm::rotate(baseMatrix, angle, glm::vec3(0, 1, 0));
    baseMatrix = glm::scale(baseMatrix, glm::vec3(TRAIN_SCALE));

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
    const float PLATFORM_Y = WHEEL_RADIUS + PLATFORM_HEIGHT / 2;  // Platform sits on top of wheels
    const float CABIN_Y = PLATFORM_Y + PLATFORM_HEIGHT / 2 + CABIN_HEIGHT / 2;  // Cabin sits on platform
    const float MOTOR_Y = PLATFORM_Y + PLATFORM_HEIGHT / 2 + MOTOR_RADIUS;  // Motor sits on platform (half inside)

    // === 1. PLATFORM (base for everything) ===
    // Default box origin is at center
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, PLATFORM_Y, 0));  // Lift to wheel height
        modelMatrix = glm::scale(modelMatrix, glm::vec3(PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }

    // === 2. CABIN (vertical box at back) ===
    // Default box: center at (0,0,0), extends +/-0.5 in each direction
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, CABIN_Y, -PLATFORM_LENGTH / 2 + CABIN_LENGTH / 2));  // Move to back
        modelMatrix = glm::scale(modelMatrix, glm::vec3(CABIN_WIDTH, CABIN_HEIGHT, CABIN_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }

    // === 3. MOTOR (horizontal cylinder at front) ===
    // Default cylinder: center at (0,0,0), axis along Z, extends +/-0.5 in Z
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, MOTOR_Y, PLATFORM_LENGTH / 2 - MOTOR_LENGTH / 2));  // Move to front
        modelMatrix = glm::scale(modelMatrix, glm::vec3(MOTOR_RADIUS, MOTOR_RADIUS, MOTOR_LENGTH));
        RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
    }

    // === 4. WHEELS (6 wheels: 3 on each side) ===
    // Default cylinder: center at (0,0,0), axis along Z
    // Rotate 90 degrees around Y to make axis along X (sideways)
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
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-PLATFORM_WIDTH / 2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }

        // Right wheel (symmetric)
        {
            glm::mat4 modelMatrix = baseMatrix;
            modelMatrix = glm::translate(modelMatrix, glm::vec3(PLATFORM_WIDTH / 2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }
    }
}

void Tema1::RenderWagon(glm::vec3 position, float angle)
{
    // === WAGON STRUCTURE ===
    // Simpler than locomotive: just a box on a platform with 4 wheels
    // Origin (0,0,0) is at the CENTER of each default shape

    const float TRAIN_SCALE = 0.6f;
    glm::mat4 baseMatrix = glm::mat4(1);
    baseMatrix = glm::translate(baseMatrix, position);
    baseMatrix = glm::rotate(baseMatrix, angle, glm::vec3(0, 1, 0));
    baseMatrix = glm::scale(baseMatrix, glm::vec3(TRAIN_SCALE));

    // === DIMENSIONS ===
    const float WHEEL_RADIUS = 0.25f;
    const float WHEEL_WIDTH = 0.15f;
    const float PLATFORM_HEIGHT = 0.1f;
    const float PLATFORM_WIDTH = 1.2f;
    const float PLATFORM_LENGTH = 1.6f;
    const float BODY_WIDTH = 1.0f;
    const float BODY_HEIGHT = 0.8f;
    const float BODY_LENGTH = 1.4f;

    // Calculate vertical positions
    const float PLATFORM_Y = WHEEL_RADIUS + PLATFORM_HEIGHT / 2;  // Platform sits on wheels
    const float BODY_Y = PLATFORM_Y + PLATFORM_HEIGHT / 2 + BODY_HEIGHT / 2;  // Body sits on platform

    // === 1. PLATFORM (base) ===
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, PLATFORM_Y, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }

    // === 2. BODY (cargo box) ===
    {
        glm::mat4 modelMatrix = baseMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, BODY_Y, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(BODY_WIDTH, BODY_HEIGHT, BODY_LENGTH));
        RenderMesh(meshes["box"], shaders["VertexColor"], modelMatrix);
    }

    // === 3. WHEELS (4 wheels: 2 on each side) ===
    // Default cylinder: center at (0,0,0), axis along Z
    // Rotate 90 degrees around Y to make axis along X (sideways)
    const float WHEEL_SPACING = PLATFORM_LENGTH / 3;  // Front and back
    float wheelZPositions[] = {
        -WHEEL_SPACING / 2,  // Back wheel
        WHEEL_SPACING / 2    // Front wheel
    };

    for (int i = 0; i < 2; i++) {
        // Left wheel
        {
            glm::mat4 modelMatrix = baseMatrix;
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-PLATFORM_WIDTH / 2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }

        // Right wheel (symmetric)
        {
            glm::mat4 modelMatrix = baseMatrix;
            modelMatrix = glm::translate(modelMatrix, glm::vec3(PLATFORM_WIDTH / 2, WHEEL_RADIUS, wheelZPositions[i]));
            modelMatrix = glm::rotate(modelMatrix, (float)(M_PI / 2), glm::vec3(0, 1, 0));  // Rotate to align along X
            modelMatrix = glm::scale(modelMatrix, glm::vec3(WHEEL_RADIUS, WHEEL_RADIUS, WHEEL_WIDTH));
            RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
        }
    }
}

// === RAIL RENDERING FUNCTIONS ===

void Tema1::RenderNormalRail(glm::vec3 start, glm::vec3 end)
{
    // Normal rail: Single black parallelepiped
    const float RAIL_WIDTH = 0.3f;
    const float RAIL_HEIGHT = 0.1f;

    glm::vec3 direction = end - start;
    float length = glm::length(direction);
    direction = glm::normalize(direction);

    // Calculate center point
    glm::vec3 center = (start + end) * 0.5f;

    // Calculate rotation angle (around Y axis)
    float angle = atan2(direction.x, direction.z);

    // Create transformation matrix
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center);
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, length));

    RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);
}

void Tema1::RenderBridgeRail(glm::vec3 start, glm::vec3 end)
{
    // Bridge rail: 4 longitudinal colored stripes (along the length)
    const float RAIL_WIDTH = 0.3f;
    const float RAIL_HEIGHT = 0.1f;
    const float STRIPE_WIDTH = RAIL_WIDTH / 4.0f;  // Divide width into 4 stripes

    glm::vec3 direction = end - start;
    float length = glm::length(direction);
    direction = glm::normalize(direction);

    // Calculate center point
    glm::vec3 center = (start + end) * 0.5f;

    // Calculate rotation angle (around Y axis)
    float angle = atan2(direction.x, direction.z);

    // Calculate perpendicular direction for stripe offset (in XZ plane)
    glm::vec3 perpendicular = glm::normalize(glm::vec3(-direction.z, 0, direction.x));

    // Render 4 stripes side by side (longitudinal)
    const char* stripeNames[] = {
        "rail_bridge_stripe1",
        "rail_bridge_stripe2",
        "rail_bridge_stripe3",
        "rail_bridge_stripe4"
    };

    for (int i = 0; i < 4; i++) {
        float offset = (i - 1.5f) * STRIPE_WIDTH;  // Center the stripes: -1.5, -0.5, 0.5, 1.5
        glm::vec3 stripeCenter = center + perpendicular * offset;

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, stripeCenter);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(STRIPE_WIDTH, RAIL_HEIGHT, length));

        RenderMesh(meshes[stripeNames[i]], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::RenderTunnelRail(glm::vec3 start, glm::vec3 end)
{
    // Tunnel rail: 4 transversal colored stripes (perpendicular to the length)
    const float RAIL_WIDTH = 0.3f;
    const float RAIL_HEIGHT = 0.1f;

    glm::vec3 direction = end - start;
    float length = glm::length(direction);
    direction = glm::normalize(direction);

    // Calculate rotation angle (around Y axis)
    float angle = atan2(direction.x, direction.z);

    // Divide the rail into 4 segments along its length
    const float SEGMENT_LENGTH = length / 4.0f;

    const char* stripeNames[] = {
        "rail_tunnel_stripe1",
        "rail_tunnel_stripe2",
        "rail_tunnel_stripe3",
        "rail_tunnel_stripe4"
    };

    // Render 4 stripes one after another (transversal)
    for (int i = 0; i < 4; i++) {
        float t = (i + 0.5f) / 4.0f;  // Center of each segment: 0.125, 0.375, 0.625, 0.875
        glm::vec3 segmentCenter = start + direction * (t * length);

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, segmentCenter);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, SEGMENT_LENGTH));

        RenderMesh(meshes[stripeNames[i]], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::RenderRail(Rail* rail)
{
    if (!rail) return;

    // NO terrain rendering under rails anymore - terrain is rendered separately
    RenderTerrainUnderRail(rail);
    // Render the rail
    // Handle junction rails specially
    if (rail->isJunction()) {
        RenderJunctionRail(rail);
        return;
    }

    // Determine terrain type based on rail position
    glm::vec3 midPoint = (rail->startPosition + rail->endPosition) * 0.5f;
    RailType terrainType = DetermineTerrainType(midPoint);

    // Render based on terrain type
    switch (terrainType) {
    case RailType::NORMAL:
        RenderNormalRail(rail->startPosition, rail->endPosition);
        break;
    case RailType::BRIDGE:
        RenderBridgeRail(rail->startPosition, rail->endPosition);
        break;
    case RailType::TUNNEL:
        RenderTunnelRail(rail->startPosition, rail->endPosition);
        break;
    default:
        RenderNormalRail(rail->startPosition, rail->endPosition);
        break;
    }
}

void Tema1::RenderRails()
{
    // Render all rails in the network
    for (Rail* rail : railNetwork) {
        RenderRail(rail);
    }
}

void Tema1::InitializeRailNetwork()
{
    // Create a rail network with ONLY straight segments (no diagonals)
    // Rails are either horizontal (East-West) or vertical (North-South)
    const float RAIL_Y = 0.05f;

    // Clear any existing rails
    for (Rail* rail : railNetwork) {
        delete rail;
    }
    railNetwork.clear();

    // ===== MAIN LOOP WITH INTERSECTIONS =====
    // Rectangular loop that crosses water and goes near mountains

    // SEGMENT 1: Start at bottom-right, going WEST
    Rail* rail1 = new Rail(
        glm::vec3(15, RAIL_Y, -10),
        glm::vec3(5, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail1);

    // SEGMENT 2: Turn NORTH (approaching river)
    Rail* rail2 = new Rail(
        glm::vec3(5, RAIL_Y, -10),
        glm::vec3(5, RAIL_Y, -4),
        RailType::NORMAL
    );
    railNetwork.push_back(rail2);
    rail1->children.push_back(rail2);

    // SEGMENT 3: BRIDGE over river (NORTH, through water Z: -4 to 4)
    Rail* rail3 = new Rail(
        glm::vec3(5, RAIL_Y, -4),
        glm::vec3(5, RAIL_Y, 4),
        RailType::BRIDGE  // Will be auto-detected as bridge
    );
    railNetwork.push_back(rail3);
    rail2->children.push_back(rail3);

    // SEGMENT 4: Continue NORTH on plains after bridge
    Rail* rail4 = new Rail(
        glm::vec3(5, RAIL_Y, 4),
        glm::vec3(5, RAIL_Y, 10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail4);
    rail3->children.push_back(rail4);

    // ===== JUNCTION 1: T-Junction (3-way) =====
    // Position: (5, 10) - player can go WEST (main) or NORTH (branch to mountains)
    Rail* junction1 = new Rail(
        glm::vec3(5, RAIL_Y, 10),
        glm::vec3(5, RAIL_Y, 10),  // Junction is a point
        RailType::JUNCTION_T
    );
    railNetwork.push_back(junction1);
    rail4->children.push_back(junction1);

    // BRANCH 1A from Junction 1: Go WEST (main path)
    Rail* rail5 = new Rail(
        glm::vec3(5, RAIL_Y, 10),
        glm::vec3(-5, RAIL_Y, 10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail5);
    junction1->children.push_back(rail5);

    // BRANCH 1B from Junction 1: Go NORTH into mountains
    Rail* railBranch1 = new Rail(
        glm::vec3(5, RAIL_Y, 10),
        glm::vec3(5, RAIL_Y, 15),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch1);
    junction1->children.push_back(railBranch1);

    // Branch 1B continues: TUNNEL through northeast mountains (going NORTH)
    // Northeast mountains: X in [4, 20], Z in [12, 20]
    Rail* railBranch2 = new Rail(
        glm::vec3(5, RAIL_Y, 15),
        glm::vec3(5, RAIL_Y, 18),
        RailType::TUNNEL  // Will be auto-detected
    );
    railNetwork.push_back(railBranch2);
    railBranch1->children.push_back(railBranch2);

    // Branch 1B: Turn WEST in mountains
    Rail* railBranch3 = new Rail(
        glm::vec3(5, RAIL_Y, 18),
        glm::vec3(-5, RAIL_Y, 18),
        RailType::TUNNEL
    );
    railNetwork.push_back(railBranch3);
    railBranch2->children.push_back(railBranch3);

    // Branch 1B: Turn SOUTH, exiting mountains (northwest mountains)
    Rail* railBranch4 = new Rail(
        glm::vec3(-5, RAIL_Y, 18),
        glm::vec3(-5, RAIL_Y, 15),
        RailType::TUNNEL
    );
    railNetwork.push_back(railBranch4);
    railBranch3->children.push_back(railBranch4);

    // Branch 1B: Continue SOUTH on plains
    Rail* railBranch5 = new Rail(
        glm::vec3(-5, RAIL_Y, 15),
        glm::vec3(-5, RAIL_Y, 10),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch5);
    railBranch4->children.push_back(railBranch5);

    // Rejoin main path at rail5 endpoint
    railBranch5->children.push_back(rail5);

    // Continue main path: SEGMENT 6 - Turn SOUTH
    Rail* rail6 = new Rail(
        glm::vec3(-5, RAIL_Y, 10),
        glm::vec3(-5, RAIL_Y, 4),
        RailType::NORMAL
    );
    railNetwork.push_back(rail6);
    rail5->children.push_back(rail6);

    // SEGMENT 7: BRIDGE over river (going SOUTH)
    Rail* rail7 = new Rail(
        glm::vec3(-5, RAIL_Y, 4),
        glm::vec3(-5, RAIL_Y, -4),
        RailType::BRIDGE
    );
    railNetwork.push_back(rail7);
    rail6->children.push_back(rail7);

    // ===== JUNCTION 2: L-Junction (2-way, 90 degrees) =====
    // Position: (-5, -4) - player can go SOUTH (main) or WEST (branch to SW mountains)
    Rail* junction2 = new Rail(
        glm::vec3(-5, RAIL_Y, -4),
        glm::vec3(-5, RAIL_Y, -4),
        RailType::JUNCTION_L
    );
    railNetwork.push_back(junction2);
    rail7->children.push_back(junction2);

    // BRANCH 2A from Junction 2: Continue SOUTH (main path)
    Rail* rail8 = new Rail(
        glm::vec3(-5, RAIL_Y, -4),
        glm::vec3(-5, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail8);
    junction2->children.push_back(rail8);

    // BRANCH 2B from Junction 2: Go WEST toward southwest mountains
    Rail* railBranch2A = new Rail(
        glm::vec3(-5, RAIL_Y, -4),
        glm::vec3(-10, RAIL_Y, -4),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch2A);
    junction2->children.push_back(railBranch2A);

    // Branch 2B: Turn SOUTH into mountains
    Rail* railBranch2B = new Rail(
        glm::vec3(-10, RAIL_Y, -4),
        glm::vec3(-10, RAIL_Y, -15),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch2B);
    railBranch2A->children.push_back(railBranch2B);

    // Branch 2B: TUNNEL through southwest mountains
    // Southwest mountains: X in [-20, -4], Z in [-20, -12]
    Rail* railBranch2C = new Rail(
        glm::vec3(-10, RAIL_Y, -15),
        glm::vec3(-10, RAIL_Y, -18),
        RailType::TUNNEL
    );
    railNetwork.push_back(railBranch2C);
    railBranch2B->children.push_back(railBranch2C);

    // Branch 2B: Turn EAST in mountains
    Rail* railBranch2D = new Rail(
        glm::vec3(-10, RAIL_Y, -18),
        glm::vec3(5, RAIL_Y, -18),
        RailType::TUNNEL
    );
    railNetwork.push_back(railBranch2D);
    railBranch2C->children.push_back(railBranch2D);

    // Branch 2B: Turn NORTH, exiting mountains (SE mountains area)
    Rail* railBranch2E = new Rail(
        glm::vec3(5, RAIL_Y, -18),
        glm::vec3(5, RAIL_Y, -15),
        RailType::TUNNEL
    );
    railNetwork.push_back(railBranch2E);
    railBranch2D->children.push_back(railBranch2E);

    // Branch 2B: Continue NORTH on plains
    Rail* railBranch2F = new Rail(
        glm::vec3(5, RAIL_Y, -15),
        glm::vec3(5, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch2F);
    railBranch2E->children.push_back(railBranch2F);

    // Branch 2B: Turn WEST to rejoin
    Rail* railBranch2G = new Rail(
        glm::vec3(5, RAIL_Y, -10),
        glm::vec3(-5, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(railBranch2G);
    railBranch2F->children.push_back(railBranch2G);

    // Connect branch back to main path
    railBranch2G->children.push_back(rail8);

    // Continue main path: SEGMENT 9 - Go EAST to complete loop
    Rail* rail9 = new Rail(
        glm::vec3(-5, RAIL_Y, -10),
        glm::vec3(5, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail9);
    rail8->children.push_back(rail9);

    // SEGMENT 10: Continue EAST
    Rail* rail10 = new Rail(
        glm::vec3(5, RAIL_Y, -10),
        glm::vec3(15, RAIL_Y, -10),
        RailType::NORMAL
    );
    railNetwork.push_back(rail10);
    rail9->children.push_back(rail10);

    // CLOSE THE LOOP - connect back to start
    rail10->children.push_back(rail1);

    // Initialize train at the start
    train.currentRail = rail1;
    train.progress = 0.0f;
    train.speed = 3.0f;  // Units per second
    train.position = rail1->startPosition;
    train.angle = CalculateTrainAngle(rail1->getDirection());
    train.stopped = false;
    train.selectedDirection = -1;
    train.queuedDirection = -1;
}

void Tema1::HandleJunctionInput(int key)
{
    if (!train.stopped || !train.currentRail) return;
    if (!train.currentRail->isJunction()) return;

    int numChildren = train.currentRail->children.size();
    if (numChildren == 0) return;

    // Get train's current direction
    glm::vec3 trainDir = glm::normalize(glm::vec3(
        sin(train.angle),
        0,
        cos(train.angle)
    ));

    int selectedIndex = -1;

    // Calculate angles of each child rail relative to train's direction
    std::vector<std::pair<int, float>> childAngles;  // index, angle

    for (int i = 0; i < numChildren; i++) {
        Rail* child = train.currentRail->children[i];
        glm::vec3 childDir = glm::normalize(child->endPosition - child->startPosition);

        // Calculate angle between train direction and child direction
        float dot = glm::dot(trainDir, childDir);
        float cross = trainDir.x * childDir.z - trainDir.z * childDir.x;
        float angle = atan2(cross, dot);

        childAngles.push_back({ i, angle });
    }

    // Interpret WASD based on train's perspective
    if (key == GLFW_KEY_W) {
        // Forward - choose rail most aligned with current direction (angle closest to 0)
        float minAngleDiff = FLT_MAX;
        for (auto& pair : childAngles) {
            float angleDiff = abs(pair.second);
            if (angleDiff < minAngleDiff) {
                minAngleDiff = angleDiff;
                selectedIndex = pair.first;
            }
        }
    }
    else if (key == GLFW_KEY_A) {
        // Left - choose rail to the left (negative angle, closest to -PI/2)
        float targetAngle = -M_PI / 2.0f;
        float minDiff = FLT_MAX;
        for (auto& pair : childAngles) {
            if (pair.second < 0) {  // Left turn
                float diff = abs(pair.second - targetAngle);
                if (diff < minDiff) {
                    minDiff = diff;
                    selectedIndex = pair.first;
                }
            }
        }
    }
    else if (key == GLFW_KEY_D) {
        // Right - choose rail to the right (positive angle, closest to PI/2)
        float targetAngle = M_PI / 2.0f;
        float minDiff = FLT_MAX;
        for (auto& pair : childAngles) {
            if (pair.second > 0) {  // Right turn
                float diff = abs(pair.second - targetAngle);
                if (diff < minDiff) {
                    minDiff = diff;
                    selectedIndex = pair.first;
                }
            }
        }
    }

    // Validate and apply selection
    if (selectedIndex >= 0 && selectedIndex < numChildren) {
        train.currentRail = train.currentRail->children[selectedIndex];
        train.progress = 0.0f;
        train.stopped = false;
        train.selectedDirection = -1;

        std::cout << "Selected direction: " << (key == GLFW_KEY_W ? "Forward" :
            key == GLFW_KEY_A ? "Left" : "Right") << std::endl;
    }
}

void Tema1::UpdateTrainMovement(float deltaTime)
{
    if (!train.currentRail || train.stopped) return;

    // Move train along current rail
    float railLength = train.currentRail->getLength();
    if (railLength == 0) {
        // This is a junction (zero-length rail)
        train.stopped = true;
        train.position = train.currentRail->startPosition;
        std::cout << "Train stopped at junction. Use W (forward), A (left), or D (right) to choose direction." << std::endl;
        return;
    }

    train.progress += (train.speed * deltaTime) / railLength;

    // Check if train reached end of rail
    if (train.progress >= 1.0f) {
        train.progress = 1.0f;
        train.position = train.currentRail->endPosition;

        // Move to next rail
        Rail* nextRail = train.currentRail->getNext();
        if (nextRail) {
            // Check if the NEXT rail is a junction
            if (nextRail->isJunction()) {
                // Move to junction and stop
                train.currentRail = nextRail;
                train.progress = 0.0f;
                train.position = nextRail->startPosition;
                train.stopped = true;
                std::cout << "Train stopped at junction. Use W (forward), A (left), or D (right) to choose direction." << std::endl;
                return;
            }
            else {
                // Move to next rail segment
                train.currentRail = nextRail;
                train.progress = 0.0f;
            }
        }
        else {
            // No next rail, stop
            train.stopped = true;
            std::cout << "End of track reached." << std::endl;
            return;
        }
    }

    // Calculate current position by interpolation
    train.position = train.currentRail->startPosition +
        (train.currentRail->endPosition - train.currentRail->startPosition) * train.progress;

    // Calculate train angle from direction
    glm::vec3 direction = train.currentRail->getDirection();
    train.angle = CalculateTrainAngle(direction);
}

RailType Tema1::DetermineTerrainType(glm::vec3 position)
{
    // Check if position is in water zone (horizontal strip)
    // Water/River zone: Z in [-4, 4]
    if (position.z >= -4.0f && position.z <= 4.0f) {
        return RailType::BRIDGE;
    }

    // Check if position is in mountain zones
    // Northwest mountains: X in [-20, -4], Z in [12, 20]
    if (position.x >= -20.0f && position.x <= -4.0f &&
        position.z >= 12.0f && position.z <= 20.0f) {
        return RailType::TUNNEL;
    }

    // Northeast mountains: X in [4, 20], Z in [12, 20]
    if (position.x >= 4.0f && position.x <= 20.0f &&
        position.z >= 12.0f && position.z <= 20.0f) {
        return RailType::TUNNEL;
    }

    // Southwest mountains: X in [-20, -4], Z in [-20, -12]
    if (position.x >= -20.0f && position.x <= -4.0f &&
        position.z >= -20.0f && position.z <= -12.0f) {
        return RailType::TUNNEL;
    }

    // Southeast mountains: X in [4, 20], Z in [-20, -12]
    if (position.x >= 4.0f && position.x <= 20.0f &&
        position.z >= -20.0f && position.z <= -12.0f) {
        return RailType::TUNNEL;
    }

    // Default: plains
    return RailType::NORMAL;
}

void Tema1::RenderJunctionRail(Rail* rail)
{
    if (!rail) return;

    const float RAIL_WIDTH = 0.3f;
    const float RAIL_HEIGHT = 0.1f;
    const float JUNCTION_SIZE = 0.8f;  // Size of junction intersection

    glm::vec3 center = rail->startPosition;  // Junction is a point

    // Render different shapes based on junction type
    if (rail->type == RailType::JUNCTION_T) {
        // T-junction: 3 arms
        // Assume arms go in 3 directions (e.g., North, East, West)
        // Render horizontal bar
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(JUNCTION_SIZE, RAIL_HEIGHT, RAIL_WIDTH));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);

        // Render vertical arm
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, JUNCTION_SIZE / 2));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);
    }
    else if (rail->type == RailType::JUNCTION_CROSS) {
        // Cross junction: 4 arms (2 perpendicular bars)
        // Horizontal bar
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(JUNCTION_SIZE, RAIL_HEIGHT, RAIL_WIDTH));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);

        // Vertical bar
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, JUNCTION_SIZE));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);
    }
    else if (rail->type == RailType::JUNCTION_L) {
        // L-junction: 2 arms at 90 degrees
        // Horizontal arm
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center + glm::vec3(JUNCTION_SIZE / 4, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(JUNCTION_SIZE / 2, RAIL_HEIGHT, RAIL_WIDTH));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);

        // Vertical arm
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, 0, JUNCTION_SIZE / 4));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, JUNCTION_SIZE / 2));
        RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::RenderFullTerrain()
{
    // Render terrain grid covering the entire map
    // Map boundaries based on your rail network: approximately X: [-20, 20], Z: [-20, 20]
    const float MAP_MIN_X = -25.0f;
    const float MAP_MAX_X = 25.0f;
    const float MAP_MIN_Z = -25.0f;
    const float MAP_MAX_Z = 25.0f;
    const float TILE_SIZE = 2.0f;  // Size of each terrain tile

    // Calculate number of tiles
    int tilesX = (int)((MAP_MAX_X - MAP_MIN_X) / TILE_SIZE);
    int tilesZ = (int)((MAP_MAX_Z - MAP_MIN_Z) / TILE_SIZE);

    // Render each tile
    for (int ix = 0; ix < tilesX; ix++) {
        for (int iz = 0; iz < tilesZ; iz++) {
            float x1 = MAP_MIN_X + ix * TILE_SIZE;
            float x2 = x1 + TILE_SIZE;
            float z1 = MAP_MIN_Z + iz * TILE_SIZE;
            float z2 = z1 + TILE_SIZE;

            // Use center point to determine terrain type
            glm::vec3 centerPoint(
                (x1 + x2) * 0.5f,
                0.0f,
                (z1 + z2) * 0.5f
            );

            RailType terrainType = DetermineTerrainType(centerPoint);

            // Determine color and Y level based on terrain type
            glm::vec3 color;
            float terrainY;

            if (terrainType == RailType::BRIDGE) {
                color = glm::vec3(0.2f, 0.4f, 0.9f);  // Blue for water
                terrainY = -0.79f;
            }
            else if (terrainType == RailType::TUNNEL) {
                color = glm::vec3(0.6f, 0.4f, 0.2f);  // Brown for mountains
                terrainY = -0.6f;
            }
            else {
                color = glm::vec3(0.2f, 0.8f, 0.2f);  // Green for plains
                terrainY = -0.8f;
            }

            // Create 4 corners of the tile
            std::vector<VertexFormat> vertices = {
                VertexFormat(glm::vec3(x1, terrainY, z1), color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
                VertexFormat(glm::vec3(x2, terrainY, z1), color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
                VertexFormat(glm::vec3(x2, terrainY, z2), color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
                VertexFormat(glm::vec3(x1, terrainY, z2), color, glm::vec3(0, 1, 0), glm::vec2(0, 1))
            };
            std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

            // Create temporary mesh and render
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

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

            Mesh tempMesh("temp_terrain_tile");
            tempMesh.InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
            RenderMesh(&tempMesh, shaders["VertexColor"], glm::mat4(1));

            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &IBO);
            glDeleteVertexArrays(1, &VAO);
        }
    }
}

void Tema1::RenderTerrainUnderRail(Rail* rail)
{
    if (!rail) return;

    // For junctions, render a small square
    if (rail->isJunction()) {
        glm::vec3 center = rail->startPosition;
        const float JUNCTION_TILE_SIZE = 0.8f;

        // Always render plains under junctions
        glm::vec3 colorPlains(0.2f, 0.8f, 0.2f);
        const float PLAINS_Y = -0.5f;

        // Create a small quad under junction
        std::vector<VertexFormat> vertices = {
            VertexFormat(glm::vec3(center.x - JUNCTION_TILE_SIZE / 2, PLAINS_Y, center.z - JUNCTION_TILE_SIZE / 2), colorPlains, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
            VertexFormat(glm::vec3(center.x + JUNCTION_TILE_SIZE / 2, PLAINS_Y, center.z - JUNCTION_TILE_SIZE / 2), colorPlains, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
            VertexFormat(glm::vec3(center.x + JUNCTION_TILE_SIZE / 2, PLAINS_Y, center.z + JUNCTION_TILE_SIZE / 2), colorPlains, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
            VertexFormat(glm::vec3(center.x - JUNCTION_TILE_SIZE / 2, PLAINS_Y, center.z + JUNCTION_TILE_SIZE / 2), colorPlains, glm::vec3(0, 1, 0), glm::vec2(0, 1))
        };
        std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

        // Create temporary mesh and render
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

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

        Mesh tempMesh("temp_junction_terrain");
        tempMesh.InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
        RenderMesh(&tempMesh, shaders["VertexColor"], glm::mat4(1));

        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        glDeleteVertexArrays(1, &VAO);
        return;
    }

    // For regular rails, render a rectangle matching rail dimensions
    glm::vec3 start = rail->startPosition;
    glm::vec3 end = rail->endPosition;
    glm::vec3 direction = glm::normalize(end - start);
    float length = glm::length(end - start);

    // Determine terrain type based on rail position
    glm::vec3 midPoint = (start + end) * 0.5f;
    RailType terrainType = DetermineTerrainType(midPoint);

    // Terrain colors and Y levels
    glm::vec3 color;
    float terrainY;

    if (terrainType == RailType::BRIDGE) {
        color = glm::vec3(0.2f, 0.4f, 0.9f);  // Blue for water
        terrainY = -0.6f;
    }
    else if (terrainType == RailType::TUNNEL) {
        color = glm::vec3(0.6f, 0.4f, 0.2f);  // Brown for mountains
        terrainY = -0.3f;
    }
    else {
        color = glm::vec3(0.2f, 0.8f, 0.2f);  // Green for plains
        terrainY = -0.5f;
    }

    // Width of terrain tile - make it slightly wider than rail
    const float TILE_WIDTH = 1.0f;

    // Calculate perpendicular direction (for width)
    glm::vec3 perpendicular = glm::normalize(glm::vec3(-direction.z, 0, direction.x));

    // Create 4 corners of the rectangle
    glm::vec3 p1 = start + perpendicular * (TILE_WIDTH / 2);
    glm::vec3 p2 = start - perpendicular * (TILE_WIDTH / 2);
    glm::vec3 p3 = end - perpendicular * (TILE_WIDTH / 2);
    glm::vec3 p4 = end + perpendicular * (TILE_WIDTH / 2);

    // Set Y coordinate to terrain level
    p1.y = terrainY;
    p2.y = terrainY;
    p3.y = terrainY;
    p4.y = terrainY;

    // Create vertices
    std::vector<VertexFormat> vertices = {
        VertexFormat(p1, color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        VertexFormat(p2, color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        VertexFormat(p3, color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        VertexFormat(p4, color, glm::vec3(0, 1, 0), glm::vec2(0, 1))
    };
    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

    // Create temporary mesh and render
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    Mesh tempMesh("temp_rail_terrain");
    tempMesh.InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    RenderMesh(&tempMesh, shaders["VertexColor"], glm::mat4(1));

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
    glDeleteVertexArrays(1, &VAO);
}

float Tema1::CalculateTrainAngle(glm::vec3 direction)
{
    // Calculate angle from direction vector
    // Direction is in XZ plane (Y is up)
    // Angle is rotation around Y axis
    // Default forward is along positive Z axis
    return atan2(direction.x, direction.z);
}

void Tema1::QueueDirectionInput(int key)
{
    // Legacy function - not needed for WASD controls
    // Could be used to queue direction before reaching junction
    // For now, just ignore
}

int Tema1::ChooseDirectionIndex(int numChildren, bool allowDefault)
{
    // Legacy function for number key selection
    // Not needed for WASD controls
    return 0;
}