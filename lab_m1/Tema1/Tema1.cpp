#include "lab_m1/Tema1/Tema1.h"
#include "components/transform.h"
#include <vector>
#include <iostream>
#include <algorithm>

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
    const float PLAIN_Y0 = -0.02f;
    const float PLAIN_Y1 = -0.015f;
    const float PLAIN_Y2 = -0.01f;
    const float PLAIN_Y3 = -0.005f;
    const float PLAIN_Y4 = 0.0f;
    const float WATER_Y = 0.008f;
    const float MOUNTAIN_Y = 0.012f;

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

    // Create terrain with different reliefs (colored quads)
    // Green = Plains (Normal rails)
    // Blue = Water/River (Bridge rails)
    // Brown = Mountains (Tunnel rails)

    // Green plains (center and most of the map)
    CreateTerrainQuad("terrain_green_1", glm::vec3(-20, PLAIN_Y0, -20), glm::vec3(20, PLAIN_Y0, -20),
        glm::vec3(20, PLAIN_Y0, 20), glm::vec3(-20, PLAIN_Y0, 20), glm::vec3(0.2f, 0.8f, 0.2f));

    // Blue water/river (horizontal strip)
    CreateTerrainQuad("terrain_blue_1", glm::vec3(-20, WATER_Y, -8), glm::vec3(20, WATER_Y, -8),
        glm::vec3(20, WATER_Y, -4), glm::vec3(-20, WATER_Y, -4), glm::vec3(0.2f, 0.4f, 0.8f));

    // Brown mountains (left side)
    CreateTerrainQuad("terrain_brown_1", glm::vec3(-20, MOUNTAIN_Y, 4), glm::vec3(-10, MOUNTAIN_Y, 4),
        glm::vec3(-10, MOUNTAIN_Y, 12), glm::vec3(-20, MOUNTAIN_Y, 12), glm::vec3(0.6f, 0.4f, 0.2f));

    // Additional terrain areas
    CreateTerrainQuad("terrain_green_2", glm::vec3(-20, PLAIN_Y1, -20), glm::vec3(20, PLAIN_Y1, -20),
        glm::vec3(20, PLAIN_Y1, -8), glm::vec3(-20, PLAIN_Y1, -8), glm::vec3(0.2f, 0.8f, 0.2f));

    CreateTerrainQuad("terrain_green_3", glm::vec3(-20, PLAIN_Y2, -4), glm::vec3(20, PLAIN_Y2, -4),
        glm::vec3(20, PLAIN_Y2, 4), glm::vec3(-20, PLAIN_Y2, 4), glm::vec3(0.2f, 0.8f, 0.2f));

    CreateTerrainQuad("terrain_green_4", glm::vec3(-10, PLAIN_Y3, 4), glm::vec3(20, PLAIN_Y3, 4),
        glm::vec3(20, PLAIN_Y3, 12), glm::vec3(-10, PLAIN_Y3, 12), glm::vec3(0.2f, 0.8f, 0.2f));

    CreateTerrainQuad("terrain_green_5", glm::vec3(-20, PLAIN_Y4, 12), glm::vec3(20, PLAIN_Y4, 12),
        glm::vec3(20, PLAIN_Y4, 20), glm::vec3(-20, PLAIN_Y4, 20), glm::vec3(0.2f, 0.8f, 0.2f));

    // Create geometric shapes for train components
    CreateBox("box", glm::vec3(0.8f, 0.4f, 0.2f));  // Orange-brown color for boxes
    CreateCylinder("cylinder", glm::vec3(0.3f, 0.3f, 0.3f));  // Dark gray for wheels/cylinders

    // Create station shapes (3 different stations)
    CreateBox("station_cube", glm::vec3(0.9f, 0.9f, 0.1f));  // Yellow-ish cube station
    CreateCylinder("station_cylinder", glm::vec3(0.8f, 0.1f, 0.1f));  // Red-ish cylinder station
    CreatePyramid("station_pyramid", glm::vec3(0.1f, 0.8f, 0.8f));  // Cyan-ish pyramid station

    // Initialize the rail network
    InitializeRailNetwork();

    // Initialize train
    train.currentRail = railNetwork[0];  // Start at first rail
    train.progress = 0.0f;
    train.speed = 2.0f;  // Default speed: 2 units per second
    train.position = train.currentRail->startPosition;
    train.angle = CalculateTrainAngle(train.currentRail->endPosition - train.currentRail->startPosition);
    train.stopped = false;
    train.selectedDirection = -1;
    train.queuedDirection = -1;
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

    // Render terrain (different colored areas)
    RenderMesh(meshes["terrain_green_1"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_blue_1"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_brown_1"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_green_2"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_green_3"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_green_4"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));
    RenderMesh(meshes["terrain_green_5"], shaders["VertexColor"], glm::vec3(0, 0, 0), glm::vec3(1));

    // Render rails with different types
    RenderRails();

    // Render stations (3 different types at different locations)
    RenderStation(glm::vec3(-10, 0, 0), 0, "cube");      // Station 1: Cube at start
    RenderStation(glm::vec3(0, 0, 0), 0, "cylinder");    // Station 2: Cylinder at junction
    RenderStation(glm::vec3(10, 0, -10), 0, "pyramid");  // Station 3: Pyramid at end

    // Render locomotive at train position (no wagon)
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
    QueueDirectionInput(key);

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

void Tema1::CreateTerrainQuad(const char* name, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 color)
{
    std::vector<VertexFormat> vertices = {
        VertexFormat(p1, color, glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        VertexFormat(p2, color, glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        VertexFormat(p3, color, glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        VertexFormat(p4, color, glm::vec3(0, 1, 0), glm::vec2(0, 1))
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
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

void Tema1::RenderStation(glm::vec3 position, float angle, const std::string& type)
{
    // Render different types of stations
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.5f, 0));  // Lift above ground
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));

    if (type == "cube") {
        // Cube station - large yellow-ish box
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
        RenderMesh(meshes["station_cube"], shaders["VertexColor"], modelMatrix);
    }
    else if (type == "cylinder") {
        // Cylinder station - tall red-ish cylinder
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
        RenderMesh(meshes["station_cylinder"], shaders["VertexColor"], modelMatrix);
    }
    else if (type == "pyramid") {
        // Pyramid station - cyan-ish pyramid
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
        RenderMesh(meshes["station_pyramid"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::RenderRails()
{
    // Slightly lift rails above terrain to avoid z-fighting flicker
    const float railLift = 0.12f;
    const float railHeight = 0.06f;
    const float railWidth = 0.12f;
    const float bridgeStripeWidth = 0.03f;

    if (meshes.find("rail_normal") == meshes.end()) {
        CreateBox("rail_normal", glm::vec3(0.1f, 0.1f, 0.1f));  // Dark gray/black
    }

    // Render all rail segments with different types based on terrain
    for (Rail* rail : railNetwork) {
        glm::vec3 start = rail->startPosition;
        glm::vec3 end = rail->endPosition;
        glm::vec3 midpoint = (start + end) / 2.0f;

        // Calculate rail properties
        glm::vec3 direction = end - start;
        float length = glm::length(direction);
        float angle = atan2(direction.x, direction.z);

        // Determine rail type based on Z coordinate (terrain type)
        std::string railType = "normal";
        if (midpoint.z >= -8 && midpoint.z <= -4) {
            railType = "bridge";  // Blue water area
        }
        else if (midpoint.x <= -10 && midpoint.z >= 4 && midpoint.z <= 12) {
            railType = "tunnel";  // Brown mountain area
        }

        if (railType == "normal") {
            // Normal rail: single black box
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, midpoint + glm::vec3(0, railLift, 0));
            modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(railWidth, railHeight, length));
            RenderMesh(meshes["rail_normal"], shaders["VertexColor"], modelMatrix);
        }
        else if (railType == "bridge") {
            // Bridge rail: 4 longitudinal colored stripes
            glm::vec3 colors[4] = {
                glm::vec3(0.8f, 0.6f, 0.4f),  // Light brown
                glm::vec3(0.6f, 0.4f, 0.2f),  // Medium brown
                glm::vec3(0.8f, 0.6f, 0.4f),  // Light brown
                glm::vec3(0.6f, 0.4f, 0.2f)   // Medium brown
            };

            for (int i = 0; i < 4; i++) {
                glm::mat4 modelMatrix = glm::mat4(1);
                float offsetX = (i - 1.5f) * bridgeStripeWidth;
                modelMatrix = glm::translate(modelMatrix, midpoint + glm::vec3(0, railLift, 0));
                modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
                modelMatrix = glm::translate(modelMatrix, glm::vec3(offsetX, 0, 0));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(bridgeStripeWidth, railHeight, length));

                std::string meshName = "rail_bridge_" + std::to_string(i);
                if (meshes.find(meshName) == meshes.end()) {
                    CreateBox(meshName.c_str(), colors[i]);
                }
                RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);
            }
        }
        else if (railType == "tunnel") {
            // Tunnel rail: 4 transversal colored stripes
            glm::vec3 colors[4] = {
                glm::vec3(0.5f, 0.5f, 0.5f),  // Light gray
                glm::vec3(0.3f, 0.3f, 0.3f),  // Dark gray
                glm::vec3(0.5f, 0.5f, 0.5f),  // Light gray
                glm::vec3(0.3f, 0.3f, 0.3f)   // Dark gray
            };

            float stripeLength = length / 4.0f;
            for (int i = 0; i < 4; i++) {
                glm::mat4 modelMatrix = glm::mat4(1);
                float offsetZ = (i - 1.5f) * stripeLength;
                modelMatrix = glm::translate(modelMatrix, midpoint + glm::vec3(0, railLift, 0));
                modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, offsetZ));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(railWidth, railHeight, stripeLength));

                std::string meshName = "rail_tunnel_" + std::to_string(i);
                if (meshes.find(meshName) == meshes.end()) {
                    CreateBox(meshName.c_str(), colors[i]);
                }
                RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);
            }
        }

        // Mark junctions in red
        if (rail->isJunction()) {
            glm::mat4 junctionMarker = glm::mat4(1);
            junctionMarker = glm::translate(junctionMarker, end + glm::vec3(0, railLift + railHeight * 2.0f, 0));
            junctionMarker = glm::scale(junctionMarker, glm::vec3(0.3f, 0.3f, 0.3f));
            CreateBox("junction_marker", glm::vec3(1, 0, 0));  // Red marker
            RenderMesh(meshes["junction_marker"], shaders["VertexColor"], junctionMarker);
        }
    }
}

// === RAIL SYSTEM IMPLEMENTATION ===

void Tema1::InitializeRailNetwork()
{
    // Create a simple rail network with junctions
    // Network layout (top view):
    //
    //      [3]---[4]
    //      /
    // [0]---[1]---[2]
    //            \
    //             [5]---[6]

    // Create rail segments
    Rail* rail0 = new Rail(glm::vec3(-10, 0, 0), glm::vec3(-5, 0, 0));    // Start segment
    Rail* rail1 = new Rail(glm::vec3(-5, 0, 0), glm::vec3(0, 0, 0));      // Approach junction 1
    Rail* rail2 = new Rail(glm::vec3(0, 0, 0), glm::vec3(5, 0, 0));       // Junction 1 -> straight
    Rail* rail3 = new Rail(glm::vec3(0, 0, 0), glm::vec3(-2, 0, 5));      // Junction 1 -> left
    Rail* rail4 = new Rail(glm::vec3(-2, 0, 5), glm::vec3(-5, 0, 10));    // Left path end
    Rail* rail5 = new Rail(glm::vec3(5, 0, 0), glm::vec3(7, 0, -5));      // Junction 2 -> right (goes over bridge)
    Rail* rail6 = new Rail(glm::vec3(7, 0, -5), glm::vec3(10, 0, -10));   // Right path end

    // Connect rails (build the network tree)
    rail0->children.push_back(rail1);

    // Junction 1: can go straight (rail2) or left (rail3)
    rail1->children.push_back(rail2);
    rail1->children.push_back(rail3);

    // After straight path, another junction
    rail2->children.push_back(rail5);
    rail2->children.push_back(rail0);  // Loop back to start

    // Left path leads to end
    rail3->children.push_back(rail4);

    // Right path leads to end
    rail5->children.push_back(rail6);

    // Store all rails in network
    railNetwork.push_back(rail0);
    railNetwork.push_back(rail1);
    railNetwork.push_back(rail2);
    railNetwork.push_back(rail3);
    railNetwork.push_back(rail4);
    railNetwork.push_back(rail5);
    railNetwork.push_back(rail6);

    std::cout << "Rail network initialized with " << railNetwork.size() << " segments" << std::endl;
    std::cout << "Controls: Arrow keys (LEFT/RIGHT/UP) to choose direction at junctions" << std::endl;
}

void Tema1::UpdateTrainMovement(float deltaTime)
{
    if (!train.currentRail) return;
    if (train.stopped) return;

    float distanceToTravel = train.speed * deltaTime;

    while (distanceToTravel > 0 && train.currentRail) {
        glm::vec3 start = train.currentRail->startPosition;
        glm::vec3 end = train.currentRail->endPosition;
        float railLength = glm::length(end - start);
        if (railLength <= 0.0001f) {
            break;
        }
        float remainingOnRail = (1.0f - train.progress) * railLength;

        if (distanceToTravel < remainingOnRail) {
            train.progress += distanceToTravel / railLength;
            distanceToTravel = 0;
        }
        else {
            distanceToTravel -= remainingOnRail;
            train.progress = 0.0f;

            if (train.currentRail->isJunction()) {
                int choice = ChooseDirectionIndex(static_cast<int>(train.currentRail->children.size()), false);
                if (choice >= 0) {
                    train.currentRail = train.currentRail->children[choice];
                    train.selectedDirection = choice;
                    train.stopped = false;
                    train.queuedDirection = -1;
                }
                else {
                    train.stopped = true;
                    train.progress = 1.0f;
                    break;
                }
            }
            else {
                // Not a junction, move to next rail automatically
                Rail* nextRail = train.currentRail->getNext();
                if (nextRail) {
                    train.currentRail = nextRail;
                    train.stopped = false;
                }
                else {
                    // End of track - stop the train
                    std::cout << "Train reached end of track!" << std::endl;
                    train.stopped = true;
                    train.progress = 1.0f;
                    break;
                }
            }
        }
    }

    // Interpolate position along current rail (linear interpolation)
    glm::vec3 start = train.currentRail->startPosition;
    glm::vec3 end = train.currentRail->endPosition;
    float clampedProgress = glm::clamp(train.progress, 0.0f, 1.0f);
    train.position = glm::mix(start, end, clampedProgress);

    // Calculate train angle based on direction
    glm::vec3 direction = end - start;
    train.angle = CalculateTrainAngle(direction);
}

void Tema1::HandleJunctionInput(int key)
{
    QueueDirectionInput(key);

    if (!train.currentRail || !train.currentRail->isJunction()) return;

    // If we are stopped (end of line), try to resume using the queued direction
    if (!train.stopped) return;

    int choice = ChooseDirectionIndex(static_cast<int>(train.currentRail->children.size()), false);
    if (choice >= 0) {
        train.currentRail = train.currentRail->children[choice];
        train.progress = 0.0f;
        train.stopped = false;
        train.selectedDirection = choice;
        train.queuedDirection = -1;
    }
}

void Tema1::QueueDirectionInput(int key)
{
    if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
        train.queuedDirection = 0;
    }
    else if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_UP) {
        train.queuedDirection = 1;
    }
    else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
        train.queuedDirection = 2;
    }
}

int Tema1::ChooseDirectionIndex(int numChildren, bool allowDefault)
{
    if (numChildren <= 0) return -1;

    if (train.queuedDirection != -1) {
        return std::min(train.queuedDirection, numChildren - 1);
    }

    if (allowDefault) {
        return 0;
    }
    return -1;
}

float Tema1::CalculateTrainAngle(glm::vec3 direction)
{
    // Calculate angle in radians from direction vector
    // atan2 gives angle from positive X axis, we want angle from positive Z axis
    return atan2(direction.x, direction.z);
}
