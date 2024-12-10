// camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern float cameraYaw;
extern float cameraPitch;
extern float cameraRadius;  // Added for zoom
extern float lastX, lastY;
extern bool firstMouse;
extern bool leftMouseButtonPressed;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);  // New
void process_keyboard(GLFWwindow* window);  // New
void updateCamera();
void setupProjection(int width, int height);

#endif