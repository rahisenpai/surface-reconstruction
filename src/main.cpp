#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "contour.h"
#include "partition.h"


int main()
{
    try
    {
        std::string ringPath = "../data/ringc1.contour";
        std::string horsePath = "../data/horsenp.contour";
        std::string pellipPath = "../data/pellip.contour";
        std::string sshapePath = "../data/sshape.contour";
        std::string ltparoPath = "../data/ltparotoidp10.contour";

        std::vector<ContourPlane> contourPlanes;
        contourPlanes = parseContourFile(ringPath);

        SpacePartitioner partitioner(contourPlanes);
        partitioner.partition();

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


        while (!glfwWindowShouldClose(window))
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            setupProjection(width, height);

            updateCamera();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // rendering the contour planes 
            renderContourPlanes(contourPlanes);
            partitioner.renderPartitions();

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