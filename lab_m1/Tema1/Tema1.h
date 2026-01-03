#pragma once

#include "components/simple_scene.h"
#include <vector>
#include "utils/glm_utils.h"
#include "lab_m1/Tema1/text_renderer.h"
#include <queue>
#include <deque>

namespace m1 { class Rail; }   // forward declare, so Rail* works here

struct TrainCar {
    m1::Rail* currentRail = nullptr;   // <-- REQUIRED (your error)
    float progress = 0.0f;

    glm::vec3 position = glm::vec3(0);
    float angle = 0.0f;

    glm::vec3 incomingDir = glm::vec3(0, 0, 1);  // <-- MUST be vec3 for glm::normalize

    float traveledDist = 0.0f;
    float distBehind = 0.0f;

    int decisionCursor = 0;
};

namespace m1
{
    // === RAIL SYSTEM STRUCTURES ===

    // Rail types based on terrain
    enum class RailType {
        NORMAL,         // Normal rails on plains (black parallelepipeds)
        BRIDGE,         // Bridge rails over water (4 longitudinal colored stripes)
        TUNNEL,         // Tunnel rails through mountains (4 transversal colored stripes)
        JUNCTION_T,     // T-junction (3 exits)
        JUNCTION_CROSS, // Cross junction (4 exits)
        JUNCTION_L      // L-junction (2 exits at 90 degrees)
    };

    // Direction enum for straight rails
    enum class RailDirection {
        NONE,         // No direction (for junctions)
        NORTH_SOUTH,  // Vertical (Z-axis)
        EAST_WEST     // Horizontal (X-axis)
    };

    // Rail segment - represents a piece of track between two points
    struct Rail {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        RailType type;                   // Visual type of rail
        RailDirection direction;         // Direction for straight rails
        std::vector<Rail*> children;     // For junctions (0, 1, or multiple children)

        Rail(glm::vec3 start, glm::vec3 end, RailType railType = RailType::NORMAL)
            : startPosition(start), endPosition(end), type(railType) {
            // Auto-detect direction based on start/end positions
            float dx = abs(end.x - start.x);
            float dz = abs(end.z - start.z);
            direction = (dx > dz) ? RailDirection::EAST_WEST : RailDirection::NORTH_SOUTH;
        }

        // Check if this is a junction (has multiple children)
        bool isJunction() const {
            return type == RailType::JUNCTION_T ||
                type == RailType::JUNCTION_CROSS ||
                type == RailType::JUNCTION_L ||
                children.size() > 1;
        }

        // Get next rail (single child or first child)
        Rail* getNext() const {
            return children.empty() ? nullptr : children[0];
        }

        // Get length of this rail segment
        float getLength() const {
            return glm::length(endPosition - startPosition);
        }

        // Get direction vector (normalized)
        glm::vec3 getDirection() const {
            return glm::normalize(endPosition - startPosition);
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
        int queuedDirection;         // Next desired direction chosen by player (-1 = none)

        // ✅ NEW: direction with which we entered the current junction
        glm::vec3 incomingDir;

        Train()
            : currentRail(nullptr),
            progress(0.0f),
            speed(1.0f),
            position(0, 0, 0),
            angle(0),
            stopped(false),
            selectedDirection(-1),
            queuedDirection(-1),
            incomingDir(0, 0, 1) {
        }
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
        void CreatePyramid(const char* name, glm::vec3 color);
        void CreateTerrainQuad(const char* name, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 color);

        // Functions to render train components
        void RenderLocomotive(glm::vec3 position, float angle);
        void RenderWagon(glm::vec3 position, float angle);

        // Rail rendering functions
        void RenderRail(Rail* rail);
        void RenderNormalRail(glm::vec3 start, glm::vec3 end);
        void RenderBridgeRail(glm::vec3 start, glm::vec3 end);
        void RenderTunnelRail(glm::vec3 start, glm::vec3 end);
        void RenderJunctionRail(Rail* rail);
        void RenderTerrainUnderRail(Rail* rail);
        void RenderFullTerrain();
        RailType DetermineTerrainType(glm::vec3 position);

        // Rail system functions
        void InitializeRailNetwork();
        void UpdateTrainMovement(float deltaTime);
        void RenderRails();
        void HandleJunctionInput(int key);
        void QueueDirectionInput(int key);
        int ChooseDirectionIndex(int numChildren, bool allowDefault);
        float CalculateTrainAngle(glm::vec3 direction);

        // Train and rail data
        Train train;
        std::vector<Rail*> railNetwork;
        // locomotive distance traveled (only while moving)
        float locoTraveledDist = 0.0f;

        // global log of *chosen exits* at junctions (only when multiple options exist)
        std::vector<Rail*> decisionLog;

        // wagons
        std::vector<TrainCar> wagons;

        // helpers
        void InitWagons(int count, float spacing);
        void UpdateWagons(float dt);

        void AdvanceCarByDistance(TrainCar& car, float deltaDist);
        // inside class Tema1
        m1::Rail* ChooseExitForCarAtJunction(TrainCar& car, m1::Rail* junction);
        void LogJunctionDecisionIfNeeded(m1::Rail* junction, m1::Rail* chosen,
            const std::vector<m1::Rail*>& validExits);


    };
};   // namespace m1


namespace transform3D
{
    inline glm::mat4 Translate(float translateX, float translateY, float translateZ)
    {
        return glm::mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            translateX, translateY, translateZ, 1
        );
    }

    inline glm::mat4 Scale(float scaleX, float scaleY, float scaleZ)
    {
        return glm::mat4(
            scaleX, 0, 0, 0,
            0, scaleY, 0, 0,
            0, 0, scaleZ, 0,
            0, 0, 0, 1
        );
    }

    inline glm::mat4 RotateOX(float radians)
    {
        return glm::mat4(
            1, 0, 0, 0,
            0, cos(radians), sin(radians), 0,
            0, -sin(radians), cos(radians), 0,
            0, 0, 0, 1
        );
    }

    inline glm::mat4 RotateOY(float radians)
    {
        return glm::mat4(
            cos(radians), 0, -sin(radians), 0,
            0, 1, 0, 0,
            sin(radians), 0, cos(radians), 0,
            0, 0, 0, 1
        );
    }

    inline glm::mat4 RotateOZ(float radians)
    {
        return glm::mat4(
            cos(radians), sin(radians), 0, 0,
            -sin(radians), cos(radians), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
}   // namespace transform3D
