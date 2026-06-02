#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include <glfw3.h>

int main()
{

    // Config
    short width = 1280;
    short height = 720;

    // GLFW Init
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Creating a system window
    GLFWwindow* window = glfwCreateWindow(width, height, "Mandelbrot Explorer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync

    // Dear ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Connecting ImGui to GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int counter = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Event handling
        glfwPollEvents();

        // Start new ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Settings");

            ImGui::Text("Mandelbrot Explorer");

            if (ImGui::Button("Click me")) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("Clicks = %d", counter);

            ImGui::Text("Performance: %.3f ms/klatke (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
        
        // Rendering ImGui
        ImGui::Render();

        // Screen cleaning (OpenGL)
        glClear(GL_COLOR_BUFFER_BIT);

        // Drawing the interface on the screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Displaying a frame on the screen
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    

}