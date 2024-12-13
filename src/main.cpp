#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "contour.h"
#include "partition.h"
#include "filesystem.h"
#include "projection.h"

// Global state variables
bool g_showConvexCells = false;
bool g_showSurfaceMeshes = false;

// Text rendering helpers
void renderText(const std::string& text, float x, float y) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0f, 0.0f, 0.0f);
    
    glRasterPos2f(x, y);
    for (const char& c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
    
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void renderHelpOverlay() {
    std::stringstream ss;
    ss << "Controls:" << std::endl
       << "Left/Right Arrow: Switch files" << std::endl
       << "1-4: Select file directly" << std::endl
       << "C: Toggle convex cells (" << (g_showConvexCells ? "ON" : "OFF") << ")" << std::endl
       << "S: Toggle surface meshes (" << (g_showSurfaceMeshes ? "ON" : "OFF") << ")" << std::endl
       << "Mouse: Look around" << std::endl
       << "Scroll: Zoom" << std::endl
       << "ESC: Exit";

    float y = 20.0f;
    std::string line;
    std::istringstream iss(ss.str());
    while (std::getline(iss, line)) {
        renderText(line, 20.0f, y);
        y += 20.0f;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_C:
                g_showConvexCells = !g_showConvexCells;
                break;
            case GLFW_KEY_S:
                g_showSurfaceMeshes = !g_showSurfaceMeshes;
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    }
}

int main() {
    try {
        // Initialize filesystem with debug output
        FileSystem fs("../data");
        if (fs.getFileCount() == 0) {
            throw std::runtime_error("No contour files found in data directory");
        }
        std::cout << "Found " << fs.getFileCount() << " contour files" << std::endl;

        // Initialize GLUT for text rendering
        int argc = 1;
        char *argv[] = {(char*)"Contour Viewer"};
        glutInit(&argc, argv);

        // Load initial contours with validation
        std::vector<ContourPlane> contourPlanes = fs.getCurrentContours();
        if (contourPlanes.empty()) {
            throw std::runtime_error("Failed to load initial contours");
        }
        std::cout << "Loaded initial file: " << fs.getCurrentFileName()
                  << " with " << contourPlanes.size() << " planes" << std::endl;

        // Initialize partitioner with validation
        SpacePartitioner* partitioner = nullptr;
        Projection* projection = nullptr;

        try {
            partitioner = new SpacePartitioner(contourPlanes);
            partitioner->partition();
            projection = new Projection(*partitioner);
        }
        catch (const std::exception& e) {
            std::cerr << "Partitioner initialization error: " << e.what() << std::endl;
            throw;
        }

        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        GLFWwindow* window = glfwCreateWindow(1280, 720, "Contour Viewer", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            delete partitioner;
            delete projection;
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // OpenGL setup
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // Set callbacks
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        double lastKeyPressTime = 0.0;
        const double keyPressDelay = 0.5;

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();

            // Handle file switching with delay and validation
            if (currentTime - lastKeyPressTime > keyPressDelay) {
                bool fileChanged = false;
                size_t oldIndex = fs.getCurrentIndex();

                try {
                    // File switching logic
                    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                        fs.nextFile();
                        fileChanged = (oldIndex != fs.getCurrentIndex());
                    }
                    else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                        fs.previousFile();
                        fileChanged = (oldIndex != fs.getCurrentIndex());
                    }
                    else {
                        for (int i = 0; i < 9 && i < fs.getFileCount(); i++) {
                            if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS) {
                                fileChanged = fs.selectFile(i);
                                break;
                            }
                        }
                    }

                    if (fileChanged) {
                        std::vector<ContourPlane> newContours = fs.getCurrentContours();
                        if (!newContours.empty()) {
                            contourPlanes = std::move(newContours);
                            delete partitioner;
                            partitioner = new SpacePartitioner(contourPlanes);
                            partitioner->partition();

                            delete projection;
                            projection = new Projection(*partitioner);

                            lastKeyPressTime = currentTime;

                            std::cout << "Switched to: " << fs.getCurrentFileName()
                                    << " (File " << fs.getCurrentIndex() + 1
                                    << "/" << fs.getFileCount() << ")" << std::endl;
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "File switching error: " << e.what() << std::endl;
                }
            }

            // Update viewport and camera
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            setupProjection(width, height);

            process_keyboard(window);
            updateCamera();

            // Render with validation
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            try {
                if (!contourPlanes.empty()) {
                    // Always render contour planes
                    renderContourPlanes(contourPlanes);

                    // Render convex cells if enabled
                    if (g_showConvexCells && partitioner->getConvexCells().size() > 0) {
                        for (const auto& cell : partitioner->getConvexCells()) {
                            partitioner->renderPolyhedron(cell);
                        }
                    }

                    // Render surface meshes if enabled
                    if (g_showSurfaceMeshes) {
                        projection->renderAllReconstructions();
                    }

                    // Render help overlay
                    renderHelpOverlay();
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Render error: " << e.what() << std::endl;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        delete partitioner;
        delete projection;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
}