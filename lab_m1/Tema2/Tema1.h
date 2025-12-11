#pragma once

#include "components/simple_scene.h"
#include <vector> 
#include "utils/glm_utils.h"
#include "lab_m1/Tema1/text_renderer.h"
#include <queue>
namespace m1
{
    struct BreakoutBlock {
        glm::vec2 pos;      // top-left corner position
        float width;
        float height;
        int hits;           
        glm::vec3 color;    
        float scale;        
        bool destroying;    
    };
    struct Cell {
        glm::vec2 pos;
        float size;
        bool highlighted;
        bool pivot=false;
        glm::vec3 color = glm::vec3(1, 1, 1);
        std::string content = ""; // "bumper", "block"
    };
    struct PanelSlot {
        bool highlighted;
        glm::vec3 color;
    };
    struct PlacedBumper {
        int gridX; 
        int gridY; 
        glm::vec3 color;
    };
    struct PlacedBlock {
        int gridX;
        int gridY;
        glm::vec3 color;
    };
    struct Slot {
        bool draw;
	};

    struct Ball {
        glm::vec2 pos;   
        glm::vec2 vel;   
        float radius;    
    };
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void DrawGrid();
        void CheckSquareClick(int mouseX, int mouseY, int button, int mods);
        void CreateBumperSemicircle();
        void CreateBumperSquare();
        void DrawBumper(int x, int y);
        void DrawLeftPanel();
        void CreateSquareAndFrame();
        bool TryPlaceBumper(int centerCol, int row);
        bool TryRemoveBumper(int centerCol, int row); 
        void DrawPlacedBumpers();
        void CreateBlock();
        void CreateCannon();
        void CreateMotor();
        void DrawBlock(int x, int y);
        void DrawCannon(int x, int y);
        void DrawMotor(int x, int y);
        bool TryPlaceBlock(int i, int j);
        void DrawPlacedBlocks();
        bool TryRemoveBlock(int i, int j);
        void SwitchToPlayMode();
        bool isPlayMode = false;
        //float structureOffsetY = 0.0f; 
        //float structureSpeed = 300.0f;  
        int mouseYPos;
        int mouseXPos;
        glm::mat3 modelMatrix;
        int squareSize = 50;
        int squareSizeG = squareSize / 2;
        int gridCols = 17;
        int gridRows = 9;
        int offsetGridX = 350;
        int offsetGridY = 50;
        //int frameThickness = 5;
        int frameSize = squareSize;
        int padding = 5;
        float frameHeight = gridCols * squareSize + padding;
        float frameWidth = gridRows * squareSize + padding;
        std::vector<std::vector<Cell>> grid;
        std::vector<PlacedBumper> placedBumpers;
        std::vector<PlacedBlock> placedBlocks;
        PanelSlot leftPanelSlots[4];
        bool isDragging = false;
        std::string draggedShape = "";
        //StartPlaing
        int minX, maxX, minY, maxY;
        std::vector<std::vector<Cell>> miniGrid;
        void FindBoundaries();
        void CopyRectangle();
        void DrawMiniGrid();
        void DrawBlockPlayer(int x, int y, glm::vec3 color);
        void DrawBumperPlayer(int x, int y, glm::vec3 color1, glm::vec3 color2);
        int width, height;
        float structureOffsetX = 0.0f;
        float structureOffsetY = 0.0f;
        float prevStructureOffsetX = 0.0f; // initialize at start
        float paddleVelX;
        //upper pannel
        void CreateStartButton();
        void DrawStartButton(int x, int y);
        float paddingPanel = 10.0f;
        float buttonSize = squareSize+squareSize/4;
        void DrawSlot(int x, int y, glm::vec3 color);
        void CreateSlot();
        int numSlots = 10;
        std::vector<Slot> slots;
        int availableSlots;
        // conditions
        bool CheckPlacementRules();
		glm::vec3 startButtonColor = glm::vec3(0, 1, 0);
        bool canPlay = false;
        bool CheckConnectivity();
        void DebugPrintGrid();
        // ball
        void CreateBallMesh(const char* name, float radius, int segments);
        void UpdateBall(Ball& ball, float dt, float windowWidth, float windowHeight);
        void CheckBallPaddleCollision(Ball& ball, float paddleVelX);
		std::vector<Ball> balls;
		bool ballLaunched = false;
        bool isBreakoutMode = false;
        // breakout blocks
        std::vector<BreakoutBlock> breakoutBlocks;
        void CreateBreakoutGrid();
        void DrawBreakoutBlocks();
        void CheckBallBlockCollision(Ball& ball);
        // lives and score
        int score = 0;
        int lives = 3;
        bool stuckBall=true;
        void CreateHeart(const char* name, glm::vec3 color);
        // Text
        gfxc::TextRenderer* textRenderer;
        float initX = 1280.0f;
        float initY = 720.0f;
    };
};   // namespace m1


namespace transform2D
{
    // Translate matrix
    inline glm::mat3 Translate(float translateX, float translateY)
    {
        // TODO(student): Implement the translation matrix
        return glm::mat3(1, 0, 0,
            0, 1, 0, translateX, translateY, 1);

    }

    // Scale matrix
    inline glm::mat3 Scale(float scaleX, float scaleY)
    {
        // TODO(student): Implement the scaling matrix
        return glm::mat3(scaleX, 0, 0, 0, scaleY, 0, 0, 0, 1);

    }

    // Rotate matrix
    inline glm::mat3 Rotate(float radians)
    {
        // TODO(student): Implement the rotation matrix
        return glm::mat3(cos(radians), -sin(radians), 0, sin(radians), cos(radians), 0, 0, 0, 1);

    }
}   // namespace transform2D
