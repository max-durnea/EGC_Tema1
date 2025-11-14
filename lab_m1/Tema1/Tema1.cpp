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
    glm::ivec2 res = window->GetResolution();
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, res.x, res.y);
    std::string fontPath = PATH_JOIN(window->props.selfDir, "src", "lab_m1", "Tema1", "Hack-Bold.ttf");
    textRenderer->Load(fontPath, 60);
    glDisable(GL_DEPTH_TEST);
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);
    
    Ball ball;
    ball.radius = 10.0f;
	balls.push_back(ball);
    CreateBreakoutGrid();
	CreateBallMesh("ball", ball.radius, 32);
    CreateBumperSemicircle();
    CreateBumperSquare();
    CreateBlock();
    CreateCannon();
    CreateMotor();
    CreateHeart("heart",glm::vec3(1,0,0));

    CreateSquareAndFrame();

    CreateStartButton();
    CreateSlot();

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

    for (int i = 0; i < 4; i++) {
        leftPanelSlots[i].highlighted = false;
        leftPanelSlots[i].color = glm::vec3(0, 0, 0);
    }
	slots.resize(numSlots);
    for (int i = 0; i < numSlots; i++) {
		slots[i].draw = true;
    }
	availableSlots = numSlots;
}

void Tema1::FrameStart() {
    if (availableSlots == numSlots) {
        startButtonColor=glm::vec3(1,0,0);
	}
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

        float totalButtonsWidth = numSlots * buttonSize;
        float totalPadding = (numSlots + 1) * paddingPanel;
        float availableSpace = res.x - offsetGridX;  // space to the right of grid
        float startX = offsetGridX + (availableSpace - totalButtonsWidth - totalPadding) / 2.0f;

        float yPos = res.y - buttonSize - paddingPanel;  // near top


        float startButtonX = res.x - buttonSize - paddingPanel;
        DrawStartButton(startButtonX, yPos);

        for (int i = 0; i < numSlots; i++) {
            if(slots[i].draw==false)
				continue;
            float x = startX + paddingPanel + i * (buttonSize + paddingPanel);
            DrawSlot(x, yPos, glm::vec3(0.1f, 0.7f, 0.1f));
        }

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
		// update ball position
        paddleVelX = (structureOffsetX - prevStructureOffsetX) / deltaTimeSeconds;
        prevStructureOffsetX = structureOffsetX;
        for (auto& ball : balls) {
			auto window_res = window->GetResolution();
			UpdateBall(ball, deltaTimeSeconds, window_res.x, window_res.y);
		}
        

        for (auto& block : breakoutBlocks) {
            if (block.destroying) {
                block.scale -= deltaTimeSeconds * 3.0f; // shrink speed
                if (block.scale <= 0.0f) {
                    block.scale = 0.0f;
                    block.destroying = false;
                }
            }
        }
        
        DrawBreakoutBlocks();
        for (int h = 0; h < lives; h++) {
            glm::mat3 modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(50 + h * 60, window->GetResolution().y - squareSize/2);
            modelMatrix *= transform2D::Scale(40, 40); 
            RenderMesh2D(meshes["heart"],modelMatrix,glm::vec3(1,0,0));
        }
        textRenderer->RenderText("SCORE: " + std::to_string(score),
            10.0f,                      // x
            window->GetResolution().y - window->GetResolution().y*670.0f/initY,       // y
            0.3f,                       // scale
            glm::vec3(1.0f, 0.9f, 0.2f));
        return;
    }
    else {

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
    float speed = 1000.0f; // pixels per second
    if (isPlayMode) {
        if (window->KeyHold(GLFW_KEY_LEFT))  structureOffsetX -= deltaTime * speed;
        if (window->KeyHold(GLFW_KEY_RIGHT)) structureOffsetX += deltaTime * speed;
    }

}

void Tema1::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_P && canPlay) {
        stuckBall = true;
        SwitchToPlayMode();
    }
    if (key == GLFW_KEY_SPACE&&stuckBall==true) {
        stuckBall = false;
        float x = (rand() % 2 == 0) ? -1.0f : 1.0f;
        glm::vec2 dir = glm::normalize(glm::vec2(x * cos(glm::radians(45.0f)), sin(glm::radians(45.0f))));
        balls[0].vel = dir * 800.0f;
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
    int mouseY_gl = window->GetResolution().y - mouseY;

    float startButtonX = window->GetResolution().x - buttonSize - paddingPanel-buttonSize/2;
    float startButtonY = window->GetResolution().y - buttonSize - paddingPanel-buttonSize/2;
	printf("Mouse click at: %d %d\n", mouseX, mouseY_gl);
	printf("Start button at: %f %f\n", startButtonX, startButtonY);
    if (mouseX >= startButtonX && mouseX <= startButtonX + buttonSize &&
        mouseY_gl >= startButtonY && mouseY_gl <= startButtonY + buttonSize)
    {
        if (canPlay && !isPlayMode)
        {
            SwitchToPlayMode();
            return;
        }
    }

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

    if (isPlayMode && !breakoutBlocks.empty()) {
        int rows = 8;
        int cols = 12;
        float spacing = 8.0f;

        float totalSpacing = spacing * (cols + 1);
        float blockWidth = (width - totalSpacing) / cols;
        float blockHeight = 20.0f;
        float startX = spacing;
        float startY = height - spacing - blockHeight;

        int idx = 0;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                if (idx < breakoutBlocks.size()) {
                    breakoutBlocks[idx].pos.x = startX + col * (blockWidth + spacing);
                    breakoutBlocks[idx].pos.y = startY - row * (blockHeight + spacing)-squareSize;
                    breakoutBlocks[idx].width = blockWidth;
                    breakoutBlocks[idx].height = blockHeight;
                    idx++;
                }
            }
        }
    }
}

void Tema1::CreateHeart(const char* name, glm::vec3 color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    vertices.push_back(VertexFormat(glm::vec3(-0.25f, 0.25f, 0), color));
    vertices.push_back(VertexFormat(glm::vec3(0.25f, 0.25f, 0), color));
    vertices.push_back(VertexFormat(glm::vec3(-0.5f, 0.0f, 0), color));
    vertices.push_back(VertexFormat(glm::vec3(0.5f, 0.0f, 0), color));
    vertices.push_back(VertexFormat(glm::vec3(0.0f, -0.5f, 0), color));

    indices = { 0, 1, 4,  
                0, 2, 4,
                1, 3, 4,
                2, 0, 1,
                1, 3, 4 };

    Mesh* heart = new Mesh(name);
    CreateMesh(name, vertices, indices);
}

void Tema1::CreateBumperSemicircle() {
   
    float width = 3.0f;   
    float height = 1.0f; 
    float radiusX = width / 2.0f;  
    float radiusY = height;        

    int segments = 20;
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;


    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0)));

    for (int i = 0; i <= segments; i++) {
        float angle = M_PI * i / segments;  
        float x = radiusX * cos(angle);     
        float y = radiusY * sin(angle);    
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
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0)),  
        VertexFormat(glm::vec3(0.5f, -0.5f, 0)),  
        VertexFormat(glm::vec3(-0.5f, 0.5f, 0)),   
        VertexFormat(glm::vec3(0.5f, 0.5f, 0))    
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
void Tema1::CreateBallMesh(const char* name, float radius, int segments = 32) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // center vertex
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0)));

    // circle vertices
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0)));
    }

    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);       
        indices.push_back(i);       
        indices.push_back(i + 1);   
    }

    CreateMesh(name, vertices, indices);
}

void Tema1::CreateMotor() {
    float size = 1.0f;
    std::vector<VertexFormat> baseVertices{
        { {-size / 2, -size, 0} },
        { {size / 2, -size, 0} },
        { {-size / 2, 0, 0} },
        { {size / 2, 0, 0} }
    };
    std::vector<unsigned int> baseIndices{ 0,1,2, 2,1,3 };
    CreateMesh("motor_base", baseVertices, baseIndices);

    std::vector<VertexFormat> flameVertices{
        { {-size / 2, 0, 0} },     
        { {size / 2, 0, 0} },       
        { {0, size, 0} }          
    };
    std::vector<unsigned int> flameIndices{ 0,1,2 };
    CreateMesh("motor_flame", flameVertices, flameIndices);
}
void Tema1::CreateCannon() {

    float width = 1.0f;


    std::vector<VertexFormat> baseVertices{
        { {-width / 2, -1.5f, 0} },
        { {width / 2, -1.5f, 0} },
        { {-width / 2, -0.5f, 0} },
        { {width / 2, -0.5f, 0} }
    };
    std::vector<unsigned int> baseIndices{ 0,1,2, 2,1,3 };
    CreateMesh("cannon_base", baseVertices, baseIndices);


    std::vector<VertexFormat> midVertices{
        { {-width / 2, -0.5f, 0} },
        { {width / 2, -0.5f, 0} },
        { {-width / 2, 0.5f, 0} },
        { {width / 2, 0.5f, 0} }
    };
    std::vector<unsigned int> midIndices{ 0,1,2, 2,1,3 };
    CreateMesh("cannon_mid", midVertices, midIndices);


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
        return;
    }
    int mouseY_gl = window->GetResolution().y - mouseY;
    printf("Mouse click at: %d %d\n", mouseX, mouseY_gl);

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

    if (!panelSlotFound) {
        bool squareFound = false;
        for (int i = 0; i < gridCols && !squareFound; i++) {
            for (int j = 0; j < gridRows && !squareFound; j++) {
                float cellX = i * squareSize + padding + offsetGridX;
                float cellY = j * squareSize + padding + offsetGridY;

                if (mouseX >= cellX && mouseX <= cellX + squareSize - padding &&
                    mouseY_gl >= cellY && mouseY_gl <= cellY + squareSize - padding) {

           
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

	if (availableSlots <= 0) {
        return false;
	}
    int leftCol = centerCol - 1;
    int rightCol = centerCol + 1;
    int topRow = row;
    if (leftCol < 0 || rightCol >= gridCols || topRow >= gridRows) {
        printf("Out of bounds: leftCol=%d, rightCol=%d, topRow=%d\n", leftCol, rightCol, topRow);
        return false;
    }

    if (grid[leftCol][row].highlighted ||
        grid[centerCol][row].highlighted ||
        grid[rightCol][row].highlighted ||
        grid[centerCol][topRow - 1].highlighted) {
        printf("Squares occupied\n");
        return false;
    }


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
    // 1x1 grid squar
    float size = 1.0f;
    float sqX = -size / 2.0f;  // -0.5 
    float sqY = -size;         // -1 to 0

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
    modelMatrix *= transform2D::Scale(buttonSize, buttonSize);
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

    RenderMesh2D(meshes["cannon_base"], modelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));    
    RenderMesh2D(meshes["cannon_mid"], modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));     
    RenderMesh2D(meshes["cannon_barrel"], modelMatrix, glm::vec3(0.7f, 0.7f, 0.7f));  
}

void Tema1::DrawMotor(int x, int y) {
    y = window->GetResolution().y - y;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Scale(squareSize, squareSize);

    RenderMesh2D(meshes["motor_base"], modelMatrix, glm::vec3(0.2f, 0.6f, 0.9f)); 
    RenderMesh2D(meshes["motor_flame"], modelMatrix, glm::vec3(1.0f, 0.4f, 0.0f)); 
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
    structureOffsetY = 0.0f;  //Reset position when switching modes

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
            /*else {
                float drawX = startX + col * squareSize + squareSize / 2;
                float drawY = startY + row * squareSize + squareSize / 2;
                DrawBlockPlayer(drawX, drawY, glm::vec3(1, 1, 1));
            }*/
        }
    }
}


// rules:
bool Tema1::CheckPlacementRules() {
    bool canPlace = true;
    
    bool hasObjects = false;
    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].content != "") {
                hasObjects = true;
                break;
            }
        }
        if (hasObjects) break;
    }
    
    if (!hasObjects) {
        startButtonColor = glm::vec3(1, 0, 0);
        canPlay = false;
        return false;
    }

    for (int i = 0; i < gridCols; i++) {
        for (int j = 0; j < gridRows; j++) {
            if (grid[i][j].pivot&&grid[i][j].content== "bumper") {
				printf("Checking bumper at [%d, %d]\n", i, j);
                //Check no bumper directly above in the 3-column area
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = i + dx;

                    // check bounds horizontally
                    if (nx < 0 || nx >= gridCols)
                        continue;

                    // scan upward
                    for (int ny = j + 1; ny < gridRows; ny++) {
                        if (grid[nx][ny].content != "") {
                            printf("Cannot place bumper at [%d,%d], blocked by cell [%d,%d]\n", i, j, nx, ny);
                            canPlace = false;
                            break; // stop scanning upward for this column
                        }
                    }

                    if (!canPlace) break; // stop checking other dx if already invalid
                }

   
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

    canPlace = CheckConnectivity()&&canPlace;
    if (!canPlace) {
        startButtonColor = glm::vec3(1, 0, 0); 
        canPlay = false;
    }
    else {
        startButtonColor = glm::vec3(0, 1, 0); 
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
    std::vector<std::vector<bool>> visited(gridCols, std::vector<bool>(gridRows, false));
    int startX = -1, startY = -1;
    for (int x = 0; x < gridCols; x++) {
        for (int y = 0; y < gridRows; y++) {
            if (grid[x][y].content != "") {
                startX = x;
                startY = y;
                break;
            }
        }
        if (startX != -1) break;
    }

    // no blocks placed 
    if (startX == -1) return true;
    std::queue<std::pair<int, int>> q;
    q.push({ startX, startY });
    visited[startX][startY] = true;

    int dx[8] = { 1, -1, 0, 0};
    int dy[8] = { 0, 0, 1, -1};

    while (!q.empty()) {
        std::pair<int, int> front = q.front();
        int x = front.first;
        int y = front.second;
        q.pop();

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            // bounds
            if (nx < 0 || nx >= gridCols|| ny < 0 || ny >= gridRows)
                continue;

            // must have content to be part of structure
            if (grid[nx][ny].content == "")
                continue;

            if (!visited[nx][ny]) {
                visited[nx][ny] = true;
                q.push({ nx, ny });
            }
        }
    }

    // Check if any cell was not visited
    for (int x = 0; x < gridCols; x++) {
        for (int y = 0; y < gridRows; y++) {
            if (grid[x][y].content != "" && !visited[x][y]) {
                return false; // disconnected
            }
        }
    }

    return true; // all connected
}

// ball

void Tema1::UpdateBall(Ball& ball, float dt, float windowWidth, float windowHeight)
{
    if (stuckBall) {
        ball.vel = glm::vec2(0,0);
        ball.pos.x = structureOffsetX+width*squareSize/2;
        ball.pos.y = structureOffsetY+height*squareSize+ball.radius;
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(ball.pos.x, ball.pos.y);
        RenderMesh2D(meshes["ball"], modelMatrix, glm::vec3(1, 0, 0));
        return;
    }
    ball.pos += ball.vel * dt;

	// Check paddle collision and block collision
    CheckBallPaddleCollision(ball, paddleVelX);
    CheckBallBlockCollision(ball); 
    // Wall collisions
    if (ball.pos.x - ball.radius < 0.0f) {
        ball.pos.x = ball.radius;
        ball.vel.x = -ball.vel.x;
    }
    if (ball.pos.x + ball.radius > windowWidth) {
        ball.pos.x = windowWidth - ball.radius;
        ball.vel.x = -ball.vel.x;
    }
    if (ball.pos.y + ball.radius > windowHeight) {
        ball.pos.y = windowHeight - ball.radius;
        ball.vel.y = -ball.vel.y-10.0f;
    }
    if (ball.pos.y - ball.radius < 0.0f) {
        lives--;
        if (lives <= 0) {
            //exit(0); // game over
            printf("GAME OVER, final score:%d\n", score);
            window->Close();
        }
        else {
            // reset ball to center
            stuckBall = true;
            ball.pos = glm::vec2(windowWidth / 2.0f, windowHeight / 2.0f);
            ball.vel = glm::vec2(0.0f, -400.0f);
        }
        
    }

    //Clamp velocity
    float speed = glm::length(ball.vel);
    float maxSpeed = 1200.0f;
    if (speed > maxSpeed) {
        ball.vel = (ball.vel / speed) * maxSpeed;
    }

    // Render
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(ball.pos.x, ball.pos.y);
    RenderMesh2D(meshes["ball"], modelMatrix, glm::vec3(1, 0, 0));
}

void Tema1::CheckBallPaddleCollision(Ball& ball, float paddleVelX) {
    if (width == 0 || height == 0) return;

    float paddleLeft = structureOffsetX;
    float paddleRight = structureOffsetX + width * squareSize;
    float paddleBottom = structureOffsetY;
    float paddleTop = structureOffsetY + height * squareSize;
    if (ball.pos.x + ball.radius < paddleLeft ||
        ball.pos.x - ball.radius > paddleRight ||
        ball.pos.y + ball.radius < paddleBottom ||
        ball.pos.y - ball.radius > paddleTop) {
        return;
    }

    // Check all cells that the ball overlaps
    int colMin = (int)((ball.pos.x - ball.radius - structureOffsetX) / squareSize);
    int colMax = (int)((ball.pos.x + ball.radius - structureOffsetX) / squareSize);
    int rowMin = (int)((ball.pos.y - ball.radius - structureOffsetY) / squareSize);
    int rowMax = (int)((ball.pos.y + ball.radius - structureOffsetY) / squareSize);

    colMin = glm::clamp(colMin, 0, width - 1);
    colMax = glm::clamp(colMax, 0, width - 1);
    rowMin = glm::clamp(rowMin, 0, height - 1);
    rowMax = glm::clamp(rowMax, 0, height - 1);

    bool collided = false;

    for (int col = colMin; col <= colMax; col++) {
        for (int row = rowMin; row <= rowMax; row++) {
            Cell& cell = miniGrid[col][row];

            // Skip empty cells
            if (cell.content == "" && !cell.highlighted) continue;

            // Cell boundaries
            float cellLeft = structureOffsetX + col * squareSize;
            float cellRight = cellLeft + squareSize;
            float cellBottom = structureOffsetY + row * squareSize;
            float cellTop = cellBottom + squareSize;

            // Find closest point on cell to ball center
            float closestX = glm::clamp(ball.pos.x, cellLeft, cellRight);
            float closestY = glm::clamp(ball.pos.y, cellBottom, cellTop);

            float distX = ball.pos.x - closestX;
            float distY = ball.pos.y - closestY;
            float distanceSquared = distX * distX + distY * distY;

            // If colliding with this cell
            if (distanceSquared < ball.radius * ball.radius) {
                collided = true;

                // Push ball out along shortest distance
                float distance = sqrt(distanceSquared);
                if (distance > 0.0001f) {
                    float overlap = ball.radius - distance;
                    ball.pos.x += (distX / distance) * overlap;
                    ball.pos.y += (distY / distance) * overlap;

                    // Reflect velocity
                    float dotProduct = ball.vel.x * distX + ball.vel.y * distY;
                    float bounceFactor = (cell.content == "bumper") ? 1.5f : 1.0f;
                    ball.vel.x -= bounceFactor * 2.0f * dotProduct * distX / (distance * distance);
                    ball.vel.y -= bounceFactor * 2.0f * dotProduct * distY / (distance * distance) - 10.0f;

                    // Add paddle velocity influence
                    ball.vel.x += paddleVelX * 0.5f;
                }
            }
        }
    }
}











void Tema1::CreateBreakoutGrid() {
    breakoutBlocks.clear();

    auto window_res = window->GetResolution();

    int rows = 8;
    int cols = 12;
    float spacing = 8.0f;

    // Scale to full screen width
    float totalSpacing = spacing * (cols + 1);
    float blockWidth = (window_res.x - totalSpacing) / cols;
    float blockHeight = 20.0f;

    // Center horizontally, start from top
    float startX = spacing;
    float startY = window_res.y - spacing - blockHeight;

    // Different color for each block
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 0.2f, 0.2f),  
        glm::vec3(1.0f, 0.5f, 0.0f),  
        glm::vec3(1.0f, 0.8f, 0.0f), 
        glm::vec3(0.2f, 1.0f, 0.2f), 
        glm::vec3(0.2f, 0.5f, 1.0f),  
        glm::vec3(0.6f, 0.2f, 1.0f),  
        glm::vec3(1.0f, 0.2f, 0.8f),  
        glm::vec3(0.2f, 1.0f, 1.0f),  
    };

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            BreakoutBlock block;
            block.pos.x = startX + col * (blockWidth + spacing);
            block.pos.y = startY - row * (blockHeight + spacing)-squareSize;
            block.width = blockWidth;
            block.height = blockHeight;
            block.hits = 2;
            block.color = colors[row % colors.size()];
            block.scale = 1.0f;
            block.destroying = false;
            breakoutBlocks.push_back(block);
        }
    }
}

void Tema1::DrawBreakoutBlocks() {
    bool remaining = false;
    for (auto& block : breakoutBlocks) {
        if (block.hits > 0 || block.destroying) {
            // Calculate center position for scaling
            float centerX = block.pos.x + block.width / 2.0f;
            float centerY = block.pos.y + block.height / 2.0f;
            
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(centerX, centerY);
            modelMatrix *= transform2D::Scale(block.scale, block.scale);
            modelMatrix *= transform2D::Translate(-block.width / 2.0f, -block.height / 2.0f);
            modelMatrix *= transform2D::Scale(block.width, block.height);
            RenderMesh2D(meshes["square"], modelMatrix, block.color);
            
            if (block.hits > 0) {
                remaining = true;
            }
        }
    }
    if (!remaining) {
        printf("GAME OVER, final score:%d\n", score);
        exit(0);
    }
}

void Tema1::CheckBallBlockCollision(Ball& ball) {
    for (auto& block : breakoutBlocks) {
        if (block.hits <= 0 || block.destroying) continue;

        float blockLeft = block.pos.x;
        float blockRight = block.pos.x + block.width;
        float blockBottom = block.pos.y;
        float blockTop = block.pos.y + block.height;
        if (ball.pos.x + ball.radius < blockLeft ||
            ball.pos.x - ball.radius > blockRight ||
            ball.pos.y + ball.radius < blockBottom ||
            ball.pos.y - ball.radius > blockTop) {
            continue;
        }

        float closestX = glm::clamp(ball.pos.x, blockLeft, blockRight);
        float closestY = glm::clamp(ball.pos.y, blockBottom, blockTop);

        float distX = ball.pos.x - closestX;
        float distY = ball.pos.y - closestY;
        float distanceSquared = distX * distX + distY * distY;
        if (distanceSquared < ball.radius * ball.radius) {
            float distance = sqrt(distanceSquared);
            if (distance > 0.0001f) {
                float overlap = ball.radius - distance;
                ball.pos.x += (distX / distance) * overlap;
                ball.pos.y += (distY / distance) * overlap;

                float dotProduct = ball.vel.x * distX + ball.vel.y * distY;
                ball.vel.x -= 2.0f * dotProduct * distX / (distance * distance);
                ball.vel.y -= 2.0f * dotProduct * distY / (distance * distance);
                block.hits--;
                if (block.hits == 1) {
                    block.color = block.color * 0.5f;
                }
                else if (block.hits <= 0) {
                    score += 100;
                    block.destroying = true;
                    block.scale = 1.0f;
                }
            }

            break; 
        }
    }
}