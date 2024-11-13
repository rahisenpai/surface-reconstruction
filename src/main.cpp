#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "contour.h"

int main() {
    try {
        std::string ringPath = "../data/ringc1.contour";
        std::string horsePath = "../data/horsenp.contour";
        std::vector<ContourPlane> planes = parseContourFile(ringPath);

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            return -1;
        }

        GLFWwindow* window = glfwCreateWindow(800, 600, "Contour Renderer", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW\n";
            return -1;
        }

        glEnable(GL_DEPTH_TEST);

        // Set the mouse callbacks
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);

        // Capture the mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        while (!glfwWindowShouldClose(window)) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            setupProjection(width, height);

            updateCamera();
            renderContourPlanes(planes);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}