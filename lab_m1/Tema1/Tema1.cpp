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
    //CreateStartButton
    CreateStartButton();
    CreateSlot();
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
    // Initialize 10 slots
	slots.resize(numSlots);
    for (int i = 0; i < numSlots; i++) {
		slots[i].draw = true;
    }
	availableSlots = numSlots;
}

void Tema1::FrameStart() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 res = window->GetResolution();
    glViewport(0, 0, res.x, res.y);
    if (isPlayMode) {
        DrawMiniGrid();
    }
    else {
        DrawGrid();
        DrawPlacedBumpers();
        DrawPlacedBlocks();
        DrawLeftPanel();

        glm::ivec2 res = window->GetResolution();

        // === CALCULATE PERFECT HORIZONTAL LAYOUT ===
        float totalButtonsWidth = numSlots * buttonSize;
        float totalPadding = (numSlots + 1) * paddingPanel;
        float availableSpace = res.x - offsetGridX;  // space to the right of grid
        float startX = offsetGridX + (availableSpace - totalButtonsWidth - totalPadding) / 2.0f;

        float yPos = res.y - buttonSize - paddingPanel;  // near top

        // === DRAW START BUTTON (rightmost) ===
        float startButtonX = res.x - buttonSize - paddingPanel;
        DrawStartButton(startButtonX, yPos);

        // === DRAW 10 SLOTS (from right to left) ===
        for (int i = 0; i < numSlots; i++) {
            if(slots[i].draw==false)
				continue;
            float x = startX + paddingPanel + i * (buttonSize + paddingPanel);
            DrawSlot(x, yPos, glm::vec3(0.1f, 0.7f, 0.1f));
        }

        // Your existing objects
        float slotWidth = offsetGridX / 2;
        float slotHeight = res.y / 4.0f;
        DrawMotor(slotWidth, slotHeight * 0.5f);
        DrawCannon(slotWidth, slotHeight * 1.5f);
        DrawBlock(slotWidth, slotHeight * 2.5f);
        DrawBumper(slotWidth, slotHeight * 3.5f - squareSize / 2);
    }
}


void Tema1::Update(float deltaTimeSeconds) {
    if (isPlayMode) {
        return;
    }
    else {
        // Edit mode - dragging objects
        if (isDragging && draggedShape != "") {
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
}

void Tema1::FrameEnd()
{

}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    float speed = 300.0f; // pixels per second
    if (isPlayMode) {
        if (window->KeyHold(GLFW_KEY_LEFT))  structureOffsetX -= deltaTime * speed;
        if (window->KeyHold(GLFW_KEY_RIGHT)) structureOffsetX += deltaTime * speed;
    }

}

void Tema1::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_P && canPlay) {
        SwitchToPlayMode();
    }
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
    camera->Update();
    // recalc dynamic sizes
    float availableWidth = width * 0.6f;
    float availableHeight = height * 0.9f;
    squareSize = std::min(availableWidth / gridCols, availableHeight / gridRows);
    padding = squareSize * 0.2f;
    buttonSize = squareSize + squareSize / 4;
    float gridWidth = gridCols * squareSize + padding;
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



void Tema1::CreateSlot() {
    std::vector<VertexFormat> vertices{
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0)),  // bottom-left
        VertexFormat(glm::vec3(0.5f, -0.5f, 0)),   // bottom-right
        VertexFormat(glm::vec3(-0.5f, 0.5f, 0)),   // top-left
        VertexFormat(glm::vec3(0.5f, 0.5f, 0))     // top-right
    };

    std::vector<unsigned int> indices{
        0, 1, 2,
        2, 1, 3
    };

    CreateMesh("slot", vertices, indices);
}


void Tema1::CreateStartButton() {
    float size = 1.0f;
    std::vector<VertexFormat> vertices{
        {{-size / 2, -size / 2,0}},
        {{size / 2,-size / 2,0}},
        {{0,0,0}},
        {{size / 2,size / 2,0}},
        {{-size / 2,size / 2,0}}
    };

    std::vector<unsigned int> indices{ 0,1,2,2,3,4,4,0,2 };
    CreateMesh("start_button", vertices, indices);

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
    if (isPlayMode) {
        // Don't allow placement in play mode
        return;
    }
    int mouseY_gl = window->GetResolution().y - mouseY;
    printf("Mouse click at: %d %d\n", mouseX, mouseY_gl);

    // Check left panel slots first
    double window_height = window->GetResolution().y;
    double rect_height = window_height / 4.0;
    double panelWidth = offsetGridX;

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
                                        // If user right-clicks (button == 2), try to remove a placed block
                    if (button == 2) {
                        if (TryRemoveBlock(i, j)) {
                            printf("Removed block at [%d, %d]\n", i, j);
                        }
                        else if (TryRemoveBumper(i, j)) {
                            printf("Removed bumper at [%d, %d]\n", i, j);
                        }
                        else {
                            printf("No block to remove at [%d, %d]\n", i, j);
                        }
                    }
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
                        if (draggedShape == "block") {
                            if (TryPlaceBlock(i, j)) {
                                printf("Block placed at grid position: [%d, %d]\n", i, j);
                                isDragging = false;
                                draggedShape = "";
                            }
                            else {
                                printf("Cannot place block at [%d, %d] - not enough space\n", i, j);
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
	if (availableSlots <= 0) {
        return false;
	}
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
        grid[centerCol][topRow - 1].highlighted) {
        printf("Squares occupied\n");
        return false;
    }

    // Mark squares as occupied
    grid[leftCol][row].highlighted = true;
    grid[leftCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);
    grid[leftCol][row].content = "bumper";

    grid[centerCol][row].highlighted = true;
    grid[centerCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);
    grid[centerCol][row].content = "bumper";
    grid[centerCol][row].pivot = true;

    grid[rightCol][row].highlighted = true;
    grid[rightCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);
    grid[rightCol][row].content = "bumper";

    grid[centerCol][topRow - 1].highlighted = true;
    grid[centerCol][topRow - 1].color = glm::vec3(0.5f, 0.5f, 0.5f);
    grid[centerCol][topRow - 1].content = "bumper";

    // Store the bumper
    PlacedBumper bumper;
    bumper.gridX = centerCol;
    bumper.gridY = row;
    bumper.color = glm::vec3(1, 1, 0);
    grid[centerCol][row].pivot = true;
    placedBumpers.push_back(bumper);
	availableSlots--;
	slots[availableSlots].draw = false;
    CheckPlacementRules();
    return true;
}
// new: remove placed bumper and clear its occupied cells
bool Tema1::TryRemoveBumper(int centerCol, int row) {
    // bounds check
    if (centerCol < 0 || centerCol >= gridCols || row < 0 || row >= gridRows) {
        return false;
    }

    // find placed bumper that matches centerCol,row
    for (auto it = placedBumpers.begin(); it != placedBumpers.end(); ++it) {
        if (it->gridX == centerCol && it->gridY == row) {
            // clear grid cells in the 3x2 area: cols centerCol-1..centerCol+1, rows row..row+1
            /*for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = 0; dy <= 1; ++dy) {
                    int gx = centerCol + dx;
                    int gy = row + dy;
                    if (gx >= 0 && gx < gridCols && gy >= 0 && gy < gridRows) {
                        if (grid[gx][gy].content == "bumper") {
                            grid[gx][gy].content = "";
                            grid[gx][gy].highlighted = false;
                            grid[gx][gy].color = glm::vec3(1, 1, 1);
                        }
                    }
                }
            }*/
            grid[centerCol - 1][row].content = "";
            grid[centerCol - 1][row].highlighted = false;
            grid[centerCol - 1][row].color = glm::vec3(1, 1, 1);
            grid[centerCol -1 ][row].pivot = false;

            grid[centerCol][row].content = "";
            grid[centerCol][row].highlighted = false;
            grid[centerCol][row].color = glm::vec3(1, 1, 1);
            grid[centerCol][row].pivot = false;

            grid[centerCol + 1][row].content = "";
            grid[centerCol + 1][row].highlighted = false;
            grid[centerCol + 1][row].color = glm::vec3(1, 1, 1);
            grid[centerCol + 1][row].pivot = false;

            grid[centerCol][row - 1].content = "";
            grid[centerCol][row - 1].highlighted = false;
            grid[centerCol][row - 1].color = glm::vec3(1, 1, 1);
            grid[centerCol][row - 1].pivot = false;
            // erase placed bumper record
            placedBumpers.erase(it);
            slots[availableSlots].draw = true;
			availableSlots++;
            CheckPlacementRules();
            return true;
        }
    }

    return false;
}
bool Tema1::TryPlaceBlock(int centerCol, int row) {
    if (availableSlots <= 0) {
		return false;
    }
    if (grid[centerCol][row].highlighted == false)
    {
        grid[centerCol][row].highlighted = true;
        grid[centerCol][row].color = glm::vec3(0.5f, 0.5f, 0.5f);
        grid[centerCol][row].content = "block";
    }
    else {
        return false;
    }
    PlacedBlock block;
    block.gridX = centerCol;
    block.gridY = row;
    block.color = glm::vec3(1, 0, 1);
    grid[centerCol][row].pivot = true;
    placedBlocks.push_back(block);
	availableSlots--;
	slots[availableSlots].draw = false;
    CheckPlacementRules();
    return true;

}

bool Tema1::TryRemoveBlock(int i, int j) {
    // bounds check
    if (i < 0 || i >= gridCols || j < 0 || j >= gridRows) {
        return false;
    }

    // Find a placed block located at the exact grid cell (i,j)
    for (auto it = placedBlocks.begin(); it != placedBlocks.end(); ++it) {
        if (it->gridX == i && it->gridY == j) {
            // remove from placedBlocks
            placedBlocks.erase(it);

            // clear the grid cell state
            grid[i][j].content = "";
            grid[i][j].highlighted = false;
            grid[i][j].color = glm::vec3(1, 1, 1);
            grid[i][j].pivot = false;
            // If your TryPlaceBlock marked additional cells (for larger shapes),
            // you should also clear those here. Example for a 1xN shape:
            // for (int yy = j; yy < j + shapeHeightInSquares && yy < gridRows; ++yy) {
            //     grid[i][yy].content = "";
            //     grid[i][yy].highlighted = false;
            //     grid[i][yy].color = glm::vec3(1,1,1);
            // }
			slots[availableSlots].draw = true;
			availableSlots++;
            CheckPlacementRules();
            return true;
        }
    }

    return false;
}

void Tema1::DrawPlacedBumpers() {
    for (const auto& bumper : placedBumpers) {
        // Calculate world position from grid position
        // The bumper's center is at the center column, bottom row
        float worldX = bumper.gridX * squareSize + padding + offsetGridX + squareSize / 2.0f - padding / 2;
        float worldY = bumper.gridY * squareSize + padding + offsetGridY - padding / 2;

        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(worldX, worldY);
        modelMatrix *= transform2D::Scale(squareSize, squareSize);

        RenderMesh2D(meshes["bumper_semi"], modelMatrix, bumper.color);
        RenderMesh2D(meshes["bumper_square"], modelMatrix, glm::vec3(0, 0, 1));
    }
}
void Tema1::DrawPlacedBlocks() {
    for (const auto& block : placedBlocks) {
        float worldX = block.gridX * squareSize + padding + offsetGridX + squareSize / 2.0f - padding / 2;
        float worldY = block.gridY * squareSize + padding + offsetGridY + squareSize / 2.0f - padding / 2;;
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(worldX, worldY);
        modelMatrix *= transform2D::Scale(squareSize, squareSize);
        RenderMesh2D(meshes["block"], modelMatrix, block.color);
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


void Tema1::DrawBumper(int x, int y) {
    y = window->GetResolution().y - y;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    // Scale by squareSize to convert from grid units to pixels
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    RenderMesh2D(meshes["bumper_semi"], modelMatrix, glm::vec3(1, 1, 0));
    RenderMesh2D(meshes["bumper_square"], modelMatrix, glm::vec3(0, 0, 1));
}
void Tema1::DrawStartButton(int x, int y) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(buttonSize, buttonSize); // Example size
    RenderMesh2D(meshes["start_button"], modelMatrix, startButtonColor);
}
void Tema1::DrawSlot(int x, int y, glm::vec3 color) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(buttonSize, buttonSize);
    RenderMesh2D(meshes["slot"], modelMatrix, color);
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
        modelMatrix *= transform2D::Scale(offsetGridX, rect_height);
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

void Tema1::DrawBumperPlayer(int x, int y, glm::vec3 color1, glm::vec3 color2) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    // Scale by squareSize to convert from grid units to pixels
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    RenderMesh2D(meshes["bumper_semi"], modelMatrix, color1);
    RenderMesh2D(meshes["bumper_square"], modelMatrix, color2);
}
void Tema1::DrawBlockPlayer(int x, int y, glm::vec3 color) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(squareSize, squareSize);
    RenderMesh2D(meshes["block"], modelMatrix, color);
}
//Playing with the structure
void Tema1::SwitchToPlayMode() {
    isPlayMode = !isPlayMode;
    structureOffsetY = 0.0f;  // Reset position when switching modes

    if (isPlayMode) {
        printf("=== PLAY MODE ===\n");
        printf("Use W/S or UP/DOWN arrows to move structure\n");
        printf("Press P to return to edit mode\n");
        FindBoundaries();
        CopyRectangle();
    }
    else {
        printf("=== EDIT MODE ===\n");
    }
}
void Tema1::FindBoundaries() {
    minX = gridCols + 1, maxX = -1, minY = gridRows + 1, maxY = -1;
    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].highlighted) {
                if (i < minX) minX = i;
                if (i > maxX) maxX = i;
                if (j < minY) minY = j;
                if (j > maxY) maxY = j;
            }
        }
    }
    printf("Boundaries - minX: %d, maxX: %d, minY: %d, maxY: %d\n", minX, maxX, minY, maxY);
}

void Tema1::CopyRectangle() {
    // if nothing selected, do nothing
    if (maxX < minX || maxY < minY) {
        printf("No highlighted cells. miniGrid is empty.\n");
        miniGrid.clear();
        return;
    }
    width = maxX - minX + 1;
    height = maxY - minY + 1;
    //alloc miniGrid
    miniGrid.clear();
    miniGrid.resize(width);
    for (int c = 0; c < width; c++) {
        miniGrid[c].resize(height);
    }
    for (int i = minX; i <= maxX; i++) {
        for (int j = minY; j <= maxY; j++) {

            int localC = i - minX;      // same
            int localR = j - minY;      // bottom row is 0
            miniGrid[localC][localR] = grid[i][j];

        }
    }


}

void Tema1::DrawMiniGrid() {
    float gridPixelWidth = width * squareSize;

    if (structureOffsetX + gridPixelWidth > window->GetResolution().x)
        structureOffsetX = window->GetResolution().x - gridPixelWidth;
    if (structureOffsetX < 0) structureOffsetX = 0;
    float startX = structureOffsetX;
    float startY = structureOffsetY;


    for (int col = 0; col < width; col++) {
        for (int row = 0; row < height; row++) {
            if (miniGrid[col][row].content == "bumper") {
                // make sure neighbors exist
                if (col > 0 && col < width - 1 && row > 0) {
                    if (miniGrid[col - 1][row].content == "bumper" &&
                        miniGrid[col + 1][row].content == "bumper" &&
                        miniGrid[col][row - 1].content == "bumper") {

                        float drawX = startX + col * squareSize + squareSize / 2;
                        float drawY = startY + row * squareSize;
                        DrawBumperPlayer(drawX, drawY, glm::vec3(1, 1, 0), glm::vec3(0, 0, 1));
                    }
                }
            }
            else if (miniGrid[col][row].highlighted) {
                float drawX = startX + col * squareSize + squareSize / 2;
                float drawY = startY + row * squareSize + squareSize / 2;
                DrawBlockPlayer(drawX, drawY, glm::vec3(1, 0, 1));
            }
            else {
                float drawX = startX + col * squareSize + squareSize / 2;
                float drawY = startY + row * squareSize + squareSize / 2;
                DrawBlockPlayer(drawX, drawY, glm::vec3(1, 1, 1));
            }
        }
    }
}


// rules:
bool Tema1::CheckPlacementRules() {
    bool canPlace = true;

    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].pivot&&grid[i][j].content== "bumper") {
				printf("Checking bumper at [%d, %d]\n", i, j);
                // 1. Check no bumper directly above in the 3-column area
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = i + dx;
                    int ny = j + 1;
                    if (nx >= 0 && nx < gridCols && ny < gridRows) {
                        if (grid[nx][ny].content == "bumper") {
                            printf("Bumper at [%d, %d] has another bumper at [%d,%d]\n",i,j,nx,ny);
                            canPlace = false;
                        }
                    }
                }

                // 2. Check no adjacent bumpers diagonally
                if (i - 1 >= 0 && j - 1 >= 0 && grid[i - 1][j - 1].content == "bumper") {
                    canPlace = false;
                    printf("Bumper at [%d,%d] has another Bumper at [%d,%d]\n", i, j, i - 1, j - 1);
                }
                    
                if (i + 1 < gridCols && j - 1 >= 0 && grid[i + 1][j - 1].content == "bumper") {
                    printf("Bumper at [%d,%d] has another Bumper at [%d,%d]\n", i, j, i + 1, j - 1);
                    canPlace = false;
                }
                    
            }
        }
    }

    // If any of the checks failed, mark the start button red
    canPlace = CheckConnectivity()&&canPlace;
    if (!canPlace) {
        startButtonColor = glm::vec3(1, 0, 0); // red
        canPlay = false;
    }
    else {
        startButtonColor = glm::vec3(0, 1, 0); // green
        canPlay = true;
    }
	DebugPrintGrid();
    return canPlace;
}
void Tema1::DebugPrintGrid()
{
    std::cout << "---- GRID ----\n";

    for (int j = gridRows - 1; j >= 0; j--) { 
        for (int i = 0; i < gridCols; i++) {
            if (grid[i][j].content == "")
                std::cout << ". ";
            else
                std::cout << grid[i][j].pivot << " ";
        }
        std::cout << "\n";
    }

    std::cout << "--------------\n";
}


bool Tema1::CheckConnectivity() {
    /*int placedBlocksCount = 0;

    // count blocks
    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].content != "") placedBlocksCount++;
        }
    }

    if (placedBlocksCount <= 1) return true; // only one block, trivially connected

    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].content != "") {
                bool hasNeighbor = false;
                int dx[8] = { -1, 1, 0, 0,-1,-1,1,1 };
                int dy[8] = { 0, 0, -1, 1,-1,1,-1,1 };
                for (int k = 0; k < 8; k++) {
                    int ni = i + dx[k];
                    int nj = j + dy[k];
                    if (ni >= 0 && ni < gridCols && nj >= 0 && nj < gridRows) {
                        if (grid[ni][nj].content != "") {
                            hasNeighbor = true;
							printf("Block at [%d, %d] has neighbor at [%d, %d]\n", i, j, ni, nj);
                            break;
                        }
                    }
                }
                if (!hasNeighbor) {
                    printf("Block at [%d, %d] doesn't have neighbors\n", i, j);
                    return false; // disconnected block found
                }
                    
            }
        }
    }*/

    return true;
}
