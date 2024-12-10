// camera.cpp
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraRadius = 10.0f;  // Initial radius
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
bool leftMouseButtonPressed = false;

const float MIN_RADIUS = 2.0f;
const float MAX_RADIUS = 50.0f;
const float ZOOM_SPEED = 0.5f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Zoom with scroll wheel
    cameraRadius -= yoffset * ZOOM_SPEED;
    cameraRadius = std::clamp(cameraRadius, MIN_RADIUS, MAX_RADIUS);
}

void process_keyboard(GLFWwindow* window) {
    // Zoom with keyboard
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        cameraRadius -= ZOOM_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        cameraRadius += ZOOM_SPEED;
    }
    cameraRadius = std::clamp(cameraRadius, MIN_RADIUS, MAX_RADIUS);
}

void updateCamera() {
    glLoadIdentity();
    float camX = cameraRadius * cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    float camY = cameraRadius * sin(glm::radians(cameraPitch));
    float camZ = cameraRadius * sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMouseButtonPressed = true;
        } else if (action == GLFW_RELEASE) {
            leftMouseButtonPressed = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (leftMouseButtonPressed) {
        float xoffset = xpos - lastX;
        float yoffset = ypos - lastY;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        cameraYaw += xoffset;
        cameraPitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (cameraPitch > 89.0f)
            cameraPitch = 89.0f;
        if (cameraPitch < -89.0f)
            cameraPitch = -89.0f;
    } else {
        lastX = xpos;
        lastY = ypos;
    }
}

void setupProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}