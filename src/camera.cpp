#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
bool leftMouseButtonPressed = false;

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

void updateCamera() {
    glLoadIdentity();
    float radius = 10.0f;
    float camX = radius * cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    float camY = radius * sin(glm::radians(cameraPitch));
    float camZ = radius * sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void setupProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}