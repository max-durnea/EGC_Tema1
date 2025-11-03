#pragma once

#include "components/simple_scene.h"
#include <vector> 
#include "utils/glm_utils.h"

namespace m1
{
    struct Cell {
        glm::vec2 pos;  
        float size;
        bool highlighted; 
        glm::vec3 color = glm::vec3(1, 1, 1);
    };
    // Add this struct definition (if you don't already have it)
    struct PanelSlot {
        bool highlighted;
        glm::vec3 color;
    };
    struct PlacedBumper {
        int gridX;  // center column position
        int gridY;  // bottom row position
        glm::vec3 color;
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
        void DrawPlacedBumpers();
        void CreateBlock();
        void CreateCannon();
        void CreateMotor();
        void DrawBlock(int x, int y);
        void DrawCannon(int x, int y);
        void DrawMotor(int x, int y);
        int mouseYPos;
        int mouseXPos;
        // TODO(student): Class variables go here
        glm::mat3 modelMatrix;
		int squareSize = 50;
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
        PanelSlot leftPanelSlots[4];
		bool isDragging = false;
		std::string draggedShape = "";

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
