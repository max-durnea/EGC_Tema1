#pragma once

#include "components/simple_scene.h"
#include <vector> 
#include "utils/glm_utils.h"
#include "lab_m1/Tema1/text_renderer.h"
#include <queue>

namespace m1
{
    // === RAIL SYSTEM STRUCTURES ===
    
    // Rail segment - represents a piece of track between two points
    struct Rail {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        std::vector<Rail*> children;  // For junctions (0, 1, or multiple children)
        
        Rail(glm::vec3 start, glm::vec3 end) 
            : startPosition(start), endPosition(end) {}
        
        // Check if this is a junction (has multiple children)
        bool isJunction() const { return children.size() > 1; }
        
        // Get next rail (single child or first child)
        Rail* getNext() const { 
            return children.empty() ? nullptr : children[0]; 
        }
    };
    
    // Train - moves along rails with interpolation
    struct Train {
        Rail* currentRail;           // Current rail segment
        float progress;              // Progress along current rail (0.0 to 1.0)
        float speed;                 // Movement speed (units per second)
        glm::vec3 position;          // Current 3D position
        float angle;                 // Current rotation angle (for direction)
        bool stopped;                // Is train stopped at junction?
        int selectedDirection;       // Which direction to take at junction (-1 = none)
        
        Train() : currentRail(nullptr), progress(0.0f), speed(1.0f), 
                  position(0, 0, 0), angle(0), stopped(false), selectedDirection(-1) {}
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
        
        // Helper functions to create geometric shapes
        void CreateBox(const char* name, glm::vec3 color);
        void CreateCylinder(const char* name, glm::vec3 color, int segments = 20);
        
        // Functions to render train components
        void RenderLocomotive(glm::vec3 position, float angle);
        
        // Rail system functions
        void InitializeRailNetwork();
        void UpdateTrainMovement(float deltaTime);
        void RenderRails();
        void HandleJunctionInput(int key);
        float CalculateTrainAngle(glm::vec3 direction);
        
        // Train and rail data
        Train train;
        std::vector<Rail*> railNetwork;  // All rail segments
    };
};   // namespace m1


namespace transform3D
{
    // Translate matrix
    inline glm::mat4 Translate(float translateX, float translateY, float translateZ)
    {
        return glm::mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            translateX, translateY, translateZ, 1
        );
    }

    // Scale matrix
    inline glm::mat4 Scale(float scaleX, float scaleY, float scaleZ)
    {
        return glm::mat4(
            scaleX, 0, 0, 0,
            0, scaleY, 0, 0,
            0, 0, scaleZ, 0,
            0, 0, 0, 1
        );
    }

    // Rotate matrix around X axis
    inline glm::mat4 RotateOX(float radians)
    {
        return glm::mat4(
            1, 0, 0, 0,
            0, cos(radians), sin(radians), 0,
            0, -sin(radians), cos(radians), 0,
            0, 0, 0, 1
        );
    }

    // Rotate matrix around Y axis
    inline glm::mat4 RotateOY(float radians)
    {
        return glm::mat4(
            cos(radians), 0, -sin(radians), 0,
            0, 1, 0, 0,
            sin(radians), 0, cos(radians), 0,
            0, 0, 0, 1
        );
    }

    // Rotate matrix around Z axis
    inline glm::mat4 RotateOZ(float radians)
    {
        return glm::mat4(
            cos(radians), sin(radians), 0, 0,
            -sin(radians), cos(radians), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
}   // namespace transform3D}   // namespace transform3D