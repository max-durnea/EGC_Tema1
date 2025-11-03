#include "lab_m1/Tema1/Tema1.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace m1;



/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{
    
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);
    // Create a square;
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
        2,3,1
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
    //initialize square grid
    for (int i = 0; i < gridRows; i++) {
        for(int j = 0; j < gridCols; j++) {
            Cell cell;
            cell.pos = glm::vec2(i * squareSize, j * squareSize);
            cell.size = squareSize;
            cell.highlighted = false;
            grid.push_back(cell);
		}
    }
}
void Tema1::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    //  Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO = 0;
    // Create the VBO and bind it
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    unsigned int IBO = 0;
    // Create the IBO and bind it
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
    // Unbind the VAO
    glBindVertexArray(0);
    glBindVertexArray(VAO);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

    // normal / other data
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, normal));

    // color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, color));

    glBindVertexArray(0);
	// create the mesh from the data
    meshes[name] = new Mesh(name);
	meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema1::FrameStart()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::Update(float deltaTimeSeconds)
{
	/*modelMatrix = glm::mat3(1);
	modelMatrix *= transform2D::Translate(50,50);
    modelMatrix *= transform2D::Scale(50, 50);
    RenderMesh2D(meshes["square"], modelMatrix, glm::vec3(0, 1, 1));
	modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(105, 50);
    modelMatrix *= transform2D::Scale(50, 50);
    RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(0, 1, 1));*/
	// Draw grid
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(i * squareSize + padding+offsetGridX, j * squareSize + padding+offsetGridY);
            modelMatrix *= transform2D::Scale(squareSize-padding,squareSize-padding);
            RenderMesh2D(meshes["square"], modelMatrix, grid[i * gridCols + j].color);
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(i * squareSize + padding + offsetGridX, j * squareSize + padding + offsetGridY);
            modelMatrix *= transform2D::Scale(squareSize-padding, squareSize-padding);
            RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(1, 0, 0));
        }
	}
    float frameHeight = gridCols * squareSize+padding;
    float frameWidth = gridRows * squareSize+padding;

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(offsetGridX, offsetGridY);
    modelMatrix *= transform2D::Scale(frameWidth, frameHeight);
    RenderMesh2D(meshes["frame"], modelMatrix, glm::vec3(0, 1, 0));
}


void Tema1::FrameEnd()
{
  
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
   

}


void Tema1::OnKeyPress(int key, int mods)
{

}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
	// when mouse is over a square, change its color

}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    int mouseY_gl =window->GetResolution().y -mouseY ;
	printf("Mouse click at: %d %d\n", mouseX, mouseY_gl);
	bool squareFound = false;
    for (int i = 0; i < gridRows&&!squareFound; i++) {
        for (int j = 0; j < gridCols&&!squareFound; j++) {
            int index = i * gridCols + j;
            float cellX = i * squareSize + padding + offsetGridX;
            float cellY = j * squareSize + padding + offsetGridY;
			printf("Checking square at row: %d, col: %d\n", i, j);
            if (mouseX >= cellX && mouseX <= cellX + squareSize - padding &&
                mouseY_gl >= cellY && mouseY_gl <= cellY + squareSize - padding) {
				printf("Square clicked at row: %d, col: %d\n", i, j);
                grid[index].highlighted = !grid[index].highlighted;
                grid[index].color = grid[index].highlighted ? glm::vec3(1, 0, 0) : glm::vec3(1, 1, 1);
				squareFound = true;
            }
        }
    }

}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
