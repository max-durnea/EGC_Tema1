#include "lab_m1/Tema1/Tema1.h"
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
    glDisable(GL_DEPTH_TEST);
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    CreateBumperSemicircle();
    CreateBumperSquare();
    CreateBlock();
    CreateCannon();
    CreateMotor();
    // Create a square and Frame
	CreateSquareAndFrame();
 

    // Initialize square grid
    grid.resize(gridCols);
    for (int r = 0; r < gridCols; r++)
        grid[r].resize(gridRows);
    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            Cell cell;
            cell.pos = glm::vec2(i * squareSize, j * squareSize);
            cell.size = squareSize;
            cell.highlighted = false;
            grid[i][j] = cell;
        }
    }

    // Initialize left panel slots
    for (int i = 0; i < 4; i++) {
        leftPanelSlots[i].highlighted = false;
        leftPanelSlots[i].color = glm::vec3(0, 0, 0);
    }
}

void Tema1::FrameStart()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);

    // Draw grid
    DrawGrid();

    // Draw placed bumpers on the grid
    DrawPlacedBumpers();

    // Draw left panel slots
    DrawLeftPanel();

    float slotWidth = (offsetGridX - 10) / 2;
    float slotHeight = window->GetResolution().y / 4.0f;

    // Draw shapes in left panel slots (centered in each slot)
    DrawMotor(slotWidth, slotHeight * 0.5f);                    // Slot 0 (bottom)
    DrawCannon(slotWidth, slotHeight * 1.5f);                   // Slot 1
    DrawBlock(slotWidth, slotHeight * 2.5f);                    // Slot 2
    DrawBumper(slotWidth, slotHeight * 3.5f - squareSize / 2);    // Slot 3 (top)
}

void Tema1::Update(float deltaTimeSeconds)
{
    if (isDragging && draggedShape != "") {
        float drawX = mouseXPos;
        float drawY = window->GetResolution().y - mouseYPos;

        if (draggedShape == "bumper") {
            DrawBumper(mouseXPos, mouseYPos + squareSize / 2);
        }
        else if (draggedShape == "block") {
            DrawBlock(mouseXPos, mouseYPos);
        }
        else if (draggedShape == "cannon") {
            DrawCannon(mouseXPos, mouseYPos);
        }
        else if (draggedShape == "motor") {
            DrawMotor(mouseXPos, mouseYPos);
        }
    }
}

void Tema1::FrameEnd()
{
    
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}

void Tema1::OnKeyPress(int key, int mods)
{
}

void Tema1::OnKeyRelease(int key, int mods)
{
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    mouseXPos = mouseX;
    mouseYPos = mouseY;
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    CheckSquareClick(mouseX, mouseY, button, mods);
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema1::OnWindowResize(int width, int height)
{
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)width, 0, (float)height, 0.01f, 400);
    // recalc dynamic sizes
    float availableWidth  = width * 0.6f;
    float availableHeight = height * 0.9f;
    squareSize = std::min(availableWidth / gridCols, availableHeight / gridRows);
    padding = squareSize * 0.2f;

    float gridWidth  = gridCols * squareSize + padding;
    float gridHeight = gridRows * squareSize + padding;

    offsetGridX = width - gridWidth - padding;
    offsetGridY = padding;
    /*CreateBumperSemicircle();
    CreateBumperSquare();
	CreateBlock();
    CreateCannon();
	CreateMotor();*/
}

void Tema1::CreateBumperSemicircle() {
    // Define in grid square units: 3 squares wide, 1 square tall
    float width = 3.0f;   // 3 grid squares wide
    float height = 1.0f;  // 1 grid square tall
    float radiusX = width / 2.0f;  // 1.5 squares
    float radiusY = height;        // 1 square

    int segments = 20;
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Center at bottom of semicircle
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0)));

    for (int i = 0; i <= segments; i++) {
        float angle = M_PI * i / segments;  // 0 to pi
        float x = radiusX * cos(angle);     // -1.5 to +1.5
        float y = radiusY * sin(angle);     // 0 to 1
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0)));
    }

    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    CreateMesh("bumper_semi", vertices, indices);
}








void Tema1::CreateMotor() {
    // Motor base (1x1 square at bottom) - colored top part
    float size = 1.0f;
    std::vector<VertexFormat> baseVertices{
        { {-size / 2, -size, 0} },
        { {size / 2, -size, 0} },
        { {-size / 2, 0, 0} },
        { {size / 2, 0, 0} }
    };
    std::vector<unsigned int> baseIndices{ 0,1,2, 2,1,3 };
    CreateMesh("motor_base", baseVertices, baseIndices);

    // Motor flame (triangle on top, 1 square tall) - orange/red flame
    std::vector<VertexFormat> flameVertices{
        { {-size / 2, 0, 0} },      // bottom left
        { {size / 2, 0, 0} },       // bottom right
        { {0, size, 0} }          // top center
    };
    std::vector<unsigned int> flameIndices{ 0,1,2 };
    CreateMesh("motor_flame", flameVertices, flameIndices);
}
void Tema1::CreateCannon() {
    // Cannon is 1x3 squares tall with multiple gray sections
    float width = 1.0f;

    // Base section (dark gray) - bottom square
    std::vector<VertexFormat> baseVertices{
        { {-width / 2, -1.5f, 0} },
        { {width / 2, -1.5f, 0} },
        { {-width / 2, -0.5f, 0} },
        { {width / 2, -0.5f, 0} }
    };
    std::vector<unsigned int> baseIndices{ 0,1,2, 2,1,3 };
    CreateMesh("cannon_base", baseVertices, baseIndices);

    // Middle section (medium gray)
    std::vector<VertexFormat> midVertices{
        { {-width / 2, -0.5f, 0} },
        { {width / 2, -0.5f, 0} },
        { {-width / 2, 0.5f, 0} },
        { {width / 2, 0.5f, 0} }
    };
    std::vector<unsigned int> midIndices{ 0,1,2, 2,1,3 };
    CreateMesh("cannon_mid", midVertices, midIndices);

    // Barrel section (light gray) - top square
    std::vector<VertexFormat> barrelVertices{
        { {-width / 2, 0.5f, 0} },
        { {width / 2, 0.5f, 0} },
        { {-width / 2, 1.5f, 0} },
        { {width / 2, 1.5f, 0} }
    };
    std::vector<unsigned int> barrelIndices{ 0,1,2, 2,1,3 };
    CreateMesh("cannon_barrel", barrelVertices, barrelIndices);
}

void Tema1::CheckSquareClick(int mouseX, int mouseY, int button, int mods) {
    int mouseY_gl = window->GetResolution().y - mouseY;
    printf("Mouse click at: %d %d\n", mouseX, mouseY_gl);

    // Check left panel slots first
    double window_height = window->GetResolution().y;
    double rect_height = window_height / 4.0;
    double panelWidth = offsetGridX - 10;

    bool panelSlotFound = false;
    for (int i = 0; i < 4 && !panelSlotFound; i++) {
        float slotX = 0;
        float slotY = i * rect_height;
        float slotEndX = panelWidth;
        float slotEndY = (i + 1) * rect_height;

        if (mouseX >= slotX && mouseX <= slotEndX &&
            mouseY_gl >= slotY && mouseY_gl <= slotEndY) {
            leftPanelSlots[i].highlighted = !leftPanelSlots[i].highlighted;
            leftPanelSlots[i].color = leftPanelSlots[i].highlighted ? glm::vec3(1, 1, 1) : glm::vec3(0, 0, 0);
            panelSlotFound = true;
            printf("Left panel slot %d clicked\n", i);
            if (i == 0) {
                isDragging = true;
                draggedShape = "bumper";
            }
            else if (i == 1)
            {
                isDragging = true;
                draggedShape = "block";
            }
            else if (i == 2)
            {
                isDragging = true;
                draggedShape = "cannon";
            }
            else if (i == 3)
            {
                isDragging = true;
                draggedShape = "motor";
            }
        }
    }

    // If no panel slot was clicked, check grid squares for bumper placement
    if (!panelSlotFound) {
        bool squareFound = false;
        for (int i = 0; i < gridCols && !squareFound; i++) {
            for (int j = 0; j < gridRows && !squareFound; j++) {
                float cellX = i * squareSize + padding + offsetGridX;
                float cellY = j * squareSize + padding + offsetGridY;

                if (mouseX >= cellX && mouseX <= cellX + squareSize - padding &&
                    mouseY_gl >= cellY && mouseY_gl <= cellY + squareSize - padding) {

                    // Try to place a bumper at this position
                    // Bumper needs: 3 squares wide (i-1, i, i+1) and 2 squares tall (j, j+1)
                    // Center column is i, so we need i-1 and i+1 to exist
                    if (isDragging)
                    {
                        if (draggedShape == "bumper") {
                            if (TryPlaceBumper(i, j)) {
                                printf("Bumper placed at grid position: [%d, %d]\n", i, j);
                                isDragging = false;
                                draggedShape = "";
                            }
                            else {
                                printf("Cannot place bumper at [%d, %d] - not enough space\n", i, j);
                            }
                            squareFound = true;
                        }
                        
                    }
                    
                }
            }
        }
    }
}
bool Tema1::TryPlaceBumper(int centerCol, int row) {
    // Bumper needs:
    // - 3 columns: centerCol-1, centerCol, centerCol+1
    // - 2 rows: row, row+1

    int leftCol = centerCol - 1;
    int rightCol = centerCol + 1;
    int topRow = row;

    // Check bounds - FIX: use correct grid dimensions
    if (leftCol < 0 || rightCol >= gridCols || topRow >= gridRows) {
        printf("Out of bounds: leftCol=%d, rightCol=%d, topRow=%d\n", leftCol, rightCol, topRow);
        return false;
    }

    // Check if all required squares are empty
    if (grid[leftCol][row].highlighted ||
        grid[centerCol][row].highlighted ||
        grid[rightCol][row].highlighted ||
        grid[centerCol][topRow-1].highlighted) {
        printf("Squares occupied\n");
        return false;
    }

    // Mark squares as occupied
    grid[leftCol][row].highlighted = true;
    grid[leftCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);

    grid[centerCol][row].highlighted = true;
    grid[centerCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);

    grid[rightCol][row].highlighted = true;
    grid[rightCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);

    grid[centerCol][topRow-1].highlighted = true;
    grid[centerCol][topRow-1].color = glm::vec3(0.5f, 0.5f, 0.5f);

    // Store the bumper
    PlacedBumper bumper;
    bumper.gridX = centerCol;
    bumper.gridY = row;
    bumper.color = glm::vec3(1, 1, 0);
    placedBumpers.push_back(bumper);

    return true;
}

void Tema1::DrawPlacedBumpers() {
    for (const auto& bumper : placedBumpers) {
        // Calculate world position from grid position
        // The bumper's center is at the center column, bottom row
        float worldX = bumper.gridX * squareSize + padding + offsetGridX + squareSize / 2.0f;
        float worldY = bumper.gridY * squareSize + padding + offsetGridY;

        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(worldX, worldY);
        modelMatrix *= transform2D::Scale(squareSize, squareSize);

        RenderMesh2D(meshes["bumper_semi"], modelMatrix, bumper.color);
        RenderMesh2D(meshes["bumper_square"], modelMatrix, glm::vec3(0, 0, 1));
    }
}



void Tema1::CreateBumperSquare() {
    // 1x1 grid square, centered horizontally under semicircle
    float size = 1.0f;
    float sqX = -size / 2.0f;  // -0.5 to center it
    float sqY = -size;         // -1 to 0, directly under semicircle

    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(VertexFormat(glm::vec3(sqX, sqY, 0)));
    vertices.push_back(VertexFormat(glm::vec3(sqX + size, sqY, 0)));
    vertices.push_back(VertexFormat(glm::vec3(sqX, sqY + size, 0)));
    vertices.push_back(VertexFormat(glm::vec3(sqX + size, sqY + size, 0)));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);

    CreateMesh("bumper_square", vertices, indices);
}
void Tema1::CreateBlock() {
    float size = 1.0f;
    std::vector<VertexFormat> vertices{
        { {-size / 2, -size / 2, 0} },
        { {size / 2, -size / 2, 0} },
        { {-size / 2, size / 2, 0} },
        { {size / 2, size / 2, 0} }
    };
    std::vector<unsigned int> indices{ 0,1,2, 2,1,3 };
    CreateMesh("block", vertices, indices);
}

void Tema1::DrawBumper(int x, int y) {
    y = window->GetResolution().y - y;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    // Scale by squareSize to convert from grid units to pixels
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    RenderMesh2D(meshes["bumper_semi"], modelMatrix, glm::vec3(1, 1, 0));
    RenderMesh2D(meshes["bumper_square"], modelMatrix, glm::vec3(0, 0, 1));
}
void Tema1::DrawBlock(int x, int y) {
    y = window->GetResolution().y - y;
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(squareSize, squareSize);
    RenderMesh2D(meshes["block"], modelMatrix, glm::vec3(1, 0, 1));
}

void Tema1::DrawCannon(int x, int y) {
    y = window->GetResolution().y - y;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    // Draw three sections with different gray shades
    RenderMesh2D(meshes["cannon_base"], modelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));    // Dark gray
    RenderMesh2D(meshes["cannon_mid"], modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));     // Medium gray
    RenderMesh2D(meshes["cannon_barrel"], modelMatrix, glm::vec3(0.7f, 0.7f, 0.7f));  // Light gray
}

void Tema1::DrawMotor(int x, int y) {
    y = window->GetResolution().y - y;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    // Draw base (colored block at top)
    RenderMesh2D(meshes["motor_base"], modelMatrix, glm::vec3(0.2f, 0.6f, 0.9f));  // Blue base
    // Draw flame (orange/red at bottom)
    RenderMesh2D(meshes["motor_flame"], modelMatrix, glm::vec3(1.0f, 0.4f, 0.0f)); // Orange flame
}

void Tema1::DrawLeftPanel() {
    double window_height = window->GetResolution().y;
    double rect_height = window_height / 4.0;

    for (int i = 0; i < 4; i++) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(0, i * rect_height);
        modelMatrix *= transform2D::Scale(offsetGridX - 9, rect_height);
        RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(0, 1, 0));
    }
}
void Tema1::DrawGrid() {
    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(i * squareSize + padding + offsetGridX, j * squareSize + padding + offsetGridY);
            modelMatrix *= transform2D::Scale(squareSize - padding, squareSize - padding);
            RenderMesh2D(meshes["square"], modelMatrix, grid[i][j].color);

            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(i * squareSize + padding + offsetGridX, j * squareSize + padding + offsetGridY);
            modelMatrix *= transform2D::Scale(squareSize - padding, squareSize - padding);
            RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(1, 0, 0));
        }
    }

    float frameHeight = gridRows * squareSize + padding;
    float frameWidth = gridCols * squareSize + padding;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(offsetGridX, offsetGridY);
    modelMatrix *= transform2D::Scale(frameWidth, frameHeight);
    RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(0, 1, 0));
}

void Tema1::CreateSquareAndFrame() {
    vector<VertexFormat> squareVertices
    {
        VertexFormat(glm::vec3(0, 0,  0)),
        VertexFormat(glm::vec3(1, 0,  0)),
        VertexFormat(glm::vec3(0, 1,  0)),
        VertexFormat(glm::vec3(1, 1,  0)),
    };
    vector<unsigned int> squareIndices =
    {
        0, 1, 2,
        2, 3, 1
    };

    vector<VertexFormat> frameVertices{
        VertexFormat(glm::vec3(0, 0,  0)),
        VertexFormat(glm::vec3(1, 0,  0)),
        VertexFormat(glm::vec3(0, 1,  0)),
        VertexFormat(glm::vec3(1, 1,  0)),
    };
    vector<unsigned int> frameIndices =
    {
        0, 1,
        1, 3,
        3, 2,
        2, 0
    };

    CreateMesh("square", squareVertices, squareIndices);
    CreateMesh("frame", frameVertices, frameIndices);
    meshes["frame"]->SetDrawMode(GL_LINES);
    printf("Hello World!\n");
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

    glBindVertexArray(0);
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, color));

    glBindVertexArray(0);

    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}