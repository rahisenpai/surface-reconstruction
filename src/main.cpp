#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "contour.h"
#include "partition.h"
#include "filesystem.h"

int main() {
    try {
        // Initialize filesystem with debug output
        FileSystem fs("../data");
        if (fs.getFileCount() == 0) {
            throw std::runtime_error("No contour files found in data directory");
        }
        std::cout << "Found " << fs.getFileCount() << " contour files" << std::endl;
        
        // Load initial contours with validation
        std::vector<ContourPlane> contourPlanes = fs.getCurrentContours();
        if (contourPlanes.empty()) {
            throw std::runtime_error("Failed to load initial contours");
        }
        std::cout << "Loaded initial file: " << fs.getCurrentFileName() 
                  << " with " << contourPlanes.size() << " planes" << std::endl;
        
        // Initialize partitioner with validation
        SpacePartitioner* partitioner = nullptr;
        try {
            partitioner = new SpacePartitioner(contourPlanes);
            // partitioner->partition();
        }
        catch (const std::exception& e) {
            std::cerr << "Partitioner initialization error: " << e.what() << std::endl;
            throw;
        }

        // GLFW initialization remains the same...
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
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // OpenGL setup
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // Set callbacks
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        double lastKeyPressTime = 0.0;
        const double keyPressDelay = 0.5; // Increased delay

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

                    // Update data if file changed
                    if (fileChanged) {
                        std::vector<ContourPlane> newContours = fs.getCurrentContours();
                        if (!newContours.empty()) {
                            contourPlanes = std::move(newContours);
                            delete partitioner;
                            partitioner = new SpacePartitioner(contourPlanes);
                            // partitioner->partition();
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
                    renderContourPlanes(contourPlanes);
                    // if (partitioner) {
                    //     partitioner->renderPartitions();
                    // }
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