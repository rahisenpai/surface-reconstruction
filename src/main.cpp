#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "contour.h"
#include "partition.h"

// Helper function to compute distance between two vertices
float distance(const Vertex& v1, const Vertex& v2) {
    return std::sqrt((v1.x - v2.x) * (v1.x - v2.x) +
                     (v1.y - v2.y) * (v1.y - v2.y) +
                     (v1.z - v2.z) * (v1.z - v2.z));
}

// Medial axis approximation using Voronoi-like approach
std::vector<std::pair<Vertex, Vertex>> computeMedialAxis(const ContourPlane& plane) {
    std::vector<std::pair<Vertex, Vertex>> medialAxisEdges;

    for (size_t i = 0; i < plane.vertices.size(); ++i) {
        for (size_t j = i + 1; j < plane.vertices.size(); ++j) {
            // Compute midpoint between vertices i and j
            Vertex mid;
            mid.x = (plane.vertices[i].x + plane.vertices[j].x) / 2;
            mid.y = (plane.vertices[i].y + plane.vertices[j].y) / 2;
            mid.z = (plane.vertices[i].z + plane.vertices[j].z) / 2;

            // Simplified filtering: Only add edges close to the vertices
            if (distance(mid, plane.vertices[i]) < 1.0f) {
                medialAxisEdges.emplace_back(plane.vertices[i], plane.vertices[j]);
            }
        }
    }

    return medialAxisEdges;
}

// Rendering function for medial axis
void renderMedialAxis(const std::vector<std::pair<Vertex, Vertex>>& medialAxisEdges) {
    glColor3f(0.0f, 0.0f, 1.0f); // Blue for medial axis
    glBegin(GL_LINES);
    for (const auto& edge : medialAxisEdges) {
        glVertex3f(edge.first.x, edge.first.y, edge.first.z);
        glVertex3f(edge.second.x, edge.second.y, edge.second.z);
    }
    glEnd();
}

int main()
{
    try
    {
        std::string ringPath = "../data/ringc1.contour";
        std::string horsePath = "../data/horsenp.contour";
        std::string pellipPath = "../data/pellip.contour";
        std::string sshapePath = "../data/sshape.contour";
        std::vector<ContourPlane> contourPlanes = parseContourFile(pellipPath);

        SpacePartitioner spacePartitioner(contourPlanes);
        std::vector<ConvexCell> cells = spacePartitioner.computeCells();

        // printing the number of cells
        std::cout << "Number of cells: " << cells.size() << "\n";


        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW\n";
            return -1;
        }

        GLFWwindow *window = glfwCreateWindow(1280, 720, "Contour Renderer", nullptr, nullptr);
        if (!window)
        {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW\n";
            return -1;
        }

        glEnable(GL_DEPTH_TEST);

        // set background color as white
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // Set the mouse callbacks
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);

        // Capture the mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        std::vector<std::vector<std::pair<Vertex, Vertex>>> medialAxisEdges;
        // Compute medial axis for all planes
        for (const auto& plane : contourPlanes) {
            medialAxisEdges.push_back(computeMedialAxis(plane));
        }

        while (!glfwWindowShouldClose(window))
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            setupProjection(width, height);

            updateCamera();

            renderContourPlanes(contourPlanes);
            spacePartitioner.renderCells();
            for (int i=0; i<contourPlanes.size(); i++) {
                renderMedialAxis(medialAxisEdges[i]);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}