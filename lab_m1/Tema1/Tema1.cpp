#include "lab_m1/Tema1/Tema1.h"
#include "components/transform.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cfloat>

using namespace std;
using namespace m1;

static bool SamePointXZ(const glm::vec3& a, const glm::vec3& b, float eps = 1e-3f)
{
    return fabs(a.x - b.x) < eps && fabs(a.z - b.z) < eps;
}

// Direction of rail r when LEAVING junctionPos
static glm::vec3 ExitDirFromJunction(m1::Rail* r, const glm::vec3& junctionPos)
{
    if (SamePointXZ(r->startPosition, junctionPos)) {
        return glm::normalize(r->endPosition - r->startPosition);
    }
    return glm::normalize(r->startPosition - r->endPosition);
}

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
            glm::vec3 junctionPos = train.currentRail->startPosition;
            Rail* next = train.currentRail->children[0];
            glm::vec3 nextDir = ExitDirFromJunction(next, junctionPos);

            train.currentRail = next;
            train.progress = 0.0f;
            train.position = junctionPos;          // ✅ spawn at junction
            train.angle = CalculateTrainAngle(nextDir);
            train.incomingDir = nextDir;           // ✅ optional, consistent
            train.stopped = false;

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

    RenderTerrainUnderRail(rail);
    
    // Handle junction rails specially
    if (rail->isJunction()) {
        RenderJunctionRail(rail);
        return;
    }

    // Use the rail's actual type instead of calculating it from position
    switch (rail->type) {
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
    const float Y = 0.05f;

    for (Rail* r : railNetwork) delete r;
    railNetwork.clear();

    auto MakeRail = [&](glm::vec3 a, glm::vec3 b, RailType t) {
        Rail* r = new Rail(a, b, t);
        railNetwork.push_back(r);
        return r;
        };

    auto MakeJunction = [&](glm::vec3 p, RailType jt) {
        Rail* j = new Rail(p, p, jt); // zero length junction
        railNetwork.push_back(j);
        return j;
        };

    // Corner points
    glm::vec3 A(-15, Y, -10);
    glm::vec3 B(-15, Y, 10);
    glm::vec3 C(15, Y, 10);
    glm::vec3 D(15, Y, -10);

    // Midpoints (T junctions)
    glm::vec3 Ttop(0, Y, 10);
    glm::vec3 Tright(15, Y, 0);
    glm::vec3 Tbot(0, Y, -10);
    glm::vec3 Tleft(-15, Y, 0);

    // Center (cross junction)
    glm::vec3 Center(0, Y, 0);

    // Junction nodes (4 Ts + 4 corners + center)
    Rail* JTop = MakeJunction(Ttop, RailType::JUNCTION_T);
    Rail* JRight = MakeJunction(Tright, RailType::JUNCTION_T);
    Rail* JBottom = MakeJunction(Tbot, RailType::JUNCTION_T);
    Rail* JLeft = MakeJunction(Tleft, RailType::JUNCTION_T);

    Rail* JA = MakeJunction(A, RailType::JUNCTION_L);
    Rail* JB = MakeJunction(B, RailType::JUNCTION_L);
    Rail* JC = MakeJunction(C, RailType::JUNCTION_L);
    Rail* JD = MakeJunction(D, RailType::JUNCTION_L);

    Rail* JCenter = MakeJunction(Center, RailType::JUNCTION_CROSS);

    // ===== Outer loop rails (CLOCKWISE) split by T junctions =====
    // Left edge:  A -> Tleft -> B
    Rail* leftB = MakeRail(A, Tleft, RailType::BRIDGE);
    Rail* leftT = MakeRail(Tleft, B, RailType::NORMAL);

    // Top edge:   B -> Ttop -> C
    Rail* topL = MakeRail(B, Ttop, RailType::BRIDGE);
    Rail* topR = MakeRail(Ttop, C, RailType::TUNNEL);

    // Right edge: C -> Tright -> D
    Rail* rightT = MakeRail(C, Tright, RailType::NORMAL);
    Rail* rightB = MakeRail(Tright, D, RailType::BRIDGE);

    // Bottom:     D -> Tbot -> A
    Rail* botR = MakeRail(D, Tbot, RailType::NORMAL);
    Rail* botL = MakeRail(Tbot, A, RailType::TUNNEL);

    // ===== Outer loop rails (COUNTERCLOCKWISE) = reverse rails =====
    Rail* leftB_r = MakeRail(Tleft, A, RailType::BRIDGE);
    Rail* leftT_r = MakeRail(B, Tleft, RailType::NORMAL);

    Rail* topL_r = MakeRail(Ttop, B, RailType::BRIDGE);
    Rail* topR_r = MakeRail(C, Ttop, RailType::TUNNEL);

    Rail* rightT_r = MakeRail(Tright, C, RailType::NORMAL);
    Rail* rightB_r = MakeRail(D, Tright, RailType::BRIDGE);

    Rail* botR_r = MakeRail(Tbot, D, RailType::NORMAL);
    Rail* botL_r = MakeRail(A, Tbot, RailType::TUNNEL);

    // ===== Spurs to/from center =====
    // T -> Center
    Rail* spurTop = MakeRail(Ttop, Center, RailType::NORMAL);
    Rail* spurRight = MakeRail(Tright, Center, RailType::NORMAL);
    Rail* spurBottom = MakeRail(Tbot, Center, RailType::NORMAL);
    Rail* spurLeft = MakeRail(Tleft, Center, RailType::NORMAL);

    // Center -> T (so progress=0 spawns at Center correctly)
    Rail* cTop = MakeRail(Center, Ttop, RailType::NORMAL);
    Rail* cRight = MakeRail(Center, Tright, RailType::NORMAL);
    Rail* cBottom = MakeRail(Center, Tbot, RailType::NORMAL);
    Rail* cLeft = MakeRail(Center, Tleft, RailType::NORMAL);

    // ===== Wire: segments -> junctions (both directions) =====

    // --- Left T junction (Tleft) ---
    leftB->children.push_back(JLeft);
    leftB_r->children.push_back(JA);            // Tleft->A goes to corner A junction
    JLeft->children.push_back(leftT);           // to B (clockwise)
    JLeft->children.push_back(leftB_r);         // to A (counterclockwise)
    JLeft->children.push_back(spurLeft);        // to Center

    // --- Top T junction (Ttop) ---
    topL->children.push_back(JTop);
    topL_r->children.push_back(JB);             // Ttop->B goes to corner B junction
    JTop->children.push_back(topR);             // to C (clockwise)
    JTop->children.push_back(topL_r);           // to B (counterclockwise)
    JTop->children.push_back(spurTop);          // to Center

    // --- Right T junction (Tright) ---
    rightT->children.push_back(JRight);
    rightT_r->children.push_back(JC);           // Tright->C goes to corner C junction
    JRight->children.push_back(rightB);         // to D (clockwise)
    JRight->children.push_back(rightT_r);       // to C (counterclockwise)
    JRight->children.push_back(spurRight);      // to Center

    // --- Bottom T junction (Tbot) ---
    botR->children.push_back(JBottom);
    botR_r->children.push_back(JD);             // Tbot->D goes to corner D junction
    JBottom->children.push_back(botL);          // to A (clockwise)
    JBottom->children.push_back(botR_r);        // to D (counterclockwise)
    JBottom->children.push_back(spurBottom);    // to Center

    // ===== Wire: corners (L junctions) so reverse direction continues =====

    // Corner A connects (A->Tleft clockwise) and (A->Tbot counterclockwise)
    botL->children.push_back(JA);               // Tbot->A arrives at A
    JA->children.push_back(leftB);              // A->Tleft
    JA->children.push_back(botL_r);             // A->Tbot (reverse of botL)

    // Corner B connects (B->Ttop clockwise) and (B->Tleft counterclockwise)
    leftT->children.push_back(JB);              // Tleft->B arrives at B
    JB->children.push_back(topL);               // B->Ttop
    JB->children.push_back(leftT_r);            // B->Tleft (reverse of leftT)

    // Corner C connects (C->Tright clockwise) and (C->Ttop counterclockwise)
    topR->children.push_back(JC);               // Ttop->C arrives at C
    JC->children.push_back(rightT);             // C->Tright
    JC->children.push_back(topR_r);             // C->Ttop (reverse of topR)

    // Corner D connects (D->Tbot clockwise) and (D->Tright counterclockwise)
    rightB->children.push_back(JD);             // Tright->D arrives at D
    JD->children.push_back(botR);               // D->Tbot
    JD->children.push_back(rightB_r);           // D->Tright (reverse of rightB)

    // ===== Wire: reverse rails reaching corners also enter the corner junctions =====
    botL_r->children.push_back(JBottom);        // A->Tbot reaches Tbot junction
    leftT_r->children.push_back(JLeft);         // B->Tleft reaches Tleft junction
    topR_r->children.push_back(JTop);           // C->Ttop reaches Ttop junction
    rightB_r->children.push_back(JRight);       // D->Tright reaches Tright junction

    // ===== Wire: spurs to/from center =====
    spurTop->children.push_back(JCenter);
    spurRight->children.push_back(JCenter);
    spurBottom->children.push_back(JCenter);
    spurLeft->children.push_back(JCenter);

    JCenter->children.push_back(cTop);
    JCenter->children.push_back(cRight);
    JCenter->children.push_back(cBottom);
    JCenter->children.push_back(cLeft);

    cTop->children.push_back(JTop);
    cRight->children.push_back(JRight);
    cBottom->children.push_back(JBottom);
    cLeft->children.push_back(JLeft);

    // ===== Train init =====
    train.currentRail = leftB; // start on loop
    train.progress = 0.0f;
    train.speed = 3.0f;
    train.position = train.currentRail->startPosition;
    train.angle = CalculateTrainAngle(train.currentRail->getDirection());
    train.stopped = false;
    train.selectedDirection = -1;
    train.queuedDirection = -1;
    train.incomingDir = train.currentRail->getDirection();
}




void Tema1::HandleJunctionInput(int key)
{
    if (!train.stopped || !train.currentRail) return;
    if (!train.currentRail->isJunction()) return;

    int numChildren = (int)train.currentRail->children.size();
    if (numChildren == 0) return;

    glm::vec3 junctionPos = train.currentRail->startPosition;
    glm::vec3 trainDir = glm::normalize(train.incomingDir);

    int selectedIndex = -1;

    // Build list of valid exits (not U-turn)
    std::vector<std::pair<int, float>> exits; // (childIndex, signedAngle)

    for (int i = 0; i < numChildren; i++) {
        Rail* child = train.currentRail->children[i];
        glm::vec3 childDir = ExitDirFromJunction(child, junctionPos);

        float dot = glm::dot(trainDir, childDir);
        float cross = trainDir.x * childDir.z - trainDir.z * childDir.x;
        float ang = atan2(cross, dot); // (-pi, pi)

        float absAng = fabs(ang);

        // Filter hard U-turns (near 180 degrees)
        if (absAng < (3.0f * (float)M_PI / 4.0f)) {
            exits.push_back({ i, ang });
        }
    }

    if (exits.empty()) {
        std::cout << ">>> No valid forward directions!\n";
        return;
    }

    if (key == GLFW_KEY_W) {
        // choose closest to straight (angle ~ 0)
        float best = FLT_MAX;
        for (auto& e : exits) {
            float d = fabs(e.second);
            if (d < best) { best = d; selectedIndex = e.first; }
        }
    }
    else if (key == GLFW_KEY_A) {
        // choose leftmost (~ -90)
        float target = -(float)M_PI / 2.0f;
        float best = FLT_MAX;
        for (auto& e : exits) {
            if (e.second < 0) {
                float d = fabs(e.second - target);
                if (d < best) { best = d; selectedIndex = e.first; }
            }
        }
        if (selectedIndex == -1) {
            std::cout << ">>> Cannot turn left from here!\n";
            return;
        }
    }
    else if (key == GLFW_KEY_D) {
        // choose rightmost (~ +90)
        float target = (float)M_PI / 2.0f;
        float best = FLT_MAX;
        for (auto& e : exits) {
            if (e.second > 0) {
                float d = fabs(e.second - target);
                if (d < best) { best = d; selectedIndex = e.first; }
            }
        }
        if (selectedIndex == -1) {
            std::cout << ">>> Cannot turn right from here!\n";
            return;
        }
    }
    else {
        return;
    }

    if (selectedIndex < 0 || selectedIndex >= numChildren) {
        std::cout << ">>> Invalid selection!\n";
        return;
    }

    Rail* selected = train.currentRail->children[selectedIndex];

    // Update angle based on direction leaving junction on that rail
    glm::vec3 chosenDir = ExitDirFromJunction(selected, junctionPos);
    train.angle = CalculateTrainAngle(chosenDir);

    train.currentRail = selected;
    train.progress = 0.0f;
    train.position = junctionPos;          // ✅ force spawn at junction point
    train.incomingDir = chosenDir;         // ✅ optional but helps consistency
    train.stopped = false;
    train.selectedDirection = -1;


    std::cout << ">>> Junction choice applied.\n";
}


void Tema1::UpdateTrainMovement(float deltaTime)
{
    if (!train.currentRail || train.stopped) return;

    float railLength = train.currentRail->getLength();

    // ===== If we are on a junction (zero-length rail) =====
    if (railLength < 1e-6f) {
        glm::vec3 junctionPos = train.currentRail->startPosition;
        glm::vec3 trainDir = glm::normalize(train.incomingDir);

        std::vector<Rail*> validExits;

        for (Rail* exit : train.currentRail->children) {
            glm::vec3 exitDir = ExitDirFromJunction(exit, junctionPos);

            float dot = glm::dot(trainDir, exitDir);
            float cross = trainDir.x * exitDir.z - trainDir.z * exitDir.x;
            float ang = atan2(cross, dot);
            float absAng = fabs(ang);

            if (absAng < (3.0f * (float)M_PI / 4.0f)) {
                validExits.push_back(exit);
            }
        }

        if (validExits.empty()) {
            train.stopped = true;
            train.position = junctionPos;
            std::cout << "Dead end at junction.\n";
            return;
        }

        if (validExits.size() == 1) {
            Rail* next = validExits[0];
            glm::vec3 nextDir = ExitDirFromJunction(next, junctionPos);
            train.angle = CalculateTrainAngle(nextDir);

            train.currentRail = next;
            train.progress = 0.0f;
            train.position = train.currentRail->startPosition;
            return;
        }

        // multiple options -> wait for player
        train.stopped = true;
        train.position = junctionPos;
        std::cout << "Stopped at junction: W forward / A left / D right\n";
        return;
    }

    // ===== Normal rail movement =====
    train.progress += (train.speed * deltaTime) / railLength;

    if (train.progress >= 1.0f) {
        train.progress = 1.0f;
        train.position = train.currentRail->endPosition;

        Rail* nextRail = train.currentRail->getNext();
        if (!nextRail) {
            train.stopped = true;
            std::cout << "End of track.\n";
            return;
        }

        // If next is junction: save incomingDir BEFORE switching
        if (nextRail->isJunction()) {
            train.incomingDir = glm::normalize(train.currentRail->endPosition - train.currentRail->startPosition);

            train.currentRail = nextRail;
            train.progress = 0.0f;
            train.position = nextRail->startPosition;

            // evaluate exits immediately (auto if single, stop if multiple)
            glm::vec3 junctionPos = train.currentRail->startPosition;
            glm::vec3 trainDir = glm::normalize(train.incomingDir);

            std::vector<Rail*> validExits;
            for (Rail* exit : train.currentRail->children) {
                glm::vec3 exitDir = ExitDirFromJunction(exit, junctionPos);
                float dot = glm::dot(trainDir, exitDir);
                float cross = trainDir.x * exitDir.z - trainDir.z * exitDir.x;
                float ang = atan2(cross, dot);
                if (fabs(ang) < (3.0f * (float)M_PI / 4.0f)) {
                    validExits.push_back(exit);
                }
            }

            if (validExits.empty()) {
                train.stopped = true;
                std::cout << "Dead end at junction.\n";
                return;
            }

            if (validExits.size() == 1) {
                Rail* only = validExits[0];
                glm::vec3 onlyDir = ExitDirFromJunction(only, junctionPos);
                train.angle = CalculateTrainAngle(onlyDir);

                train.currentRail = only;
                train.progress = 0.0f;
                train.stopped = false;
                return;
            }

            train.stopped = true;
            std::cout << "Stopped at junction: W/A/D.\n";
            return;
        }

        // Normal transition to next segment
        train.currentRail = nextRail;
        train.progress = 0.0f;
    }

    // Interpolate position
    train.position = train.currentRail->startPosition +
        (train.currentRail->endPosition - train.currentRail->startPosition) * train.progress;

    // Update angle from current rail direction
    glm::vec3 dir = train.currentRail->getDirection();
    train.angle = CalculateTrainAngle(dir);
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

    // Use rail->type instead of DetermineTerrainType()
    RailType terrainType = rail->type;

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