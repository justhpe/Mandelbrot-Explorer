#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include <glfw3.h>

int mandelbrot(double cr, double cj, int max_iterations) {
    double zr = 0.0; // Re part
    double zj = 0.0; // Im part
    int count = 0;

    // |z| < 2   --->   zr^2 + zj^2 < 4
    while (zr * zr + zj * zj <= 4.0 && count < max_iterations) {
        // (zr + zj*j)^2 = zr^2 - zj^2 + 2*zr*zj*j
        double temp = zr * zr - zj * zj + cr;
        zj = 2.0 * zr * zj + cj;
        zr = temp;
        count++;
    }

    return count;
}

int main()
{

    // Config
    short width = 1280;
    short height = 720;
    float scale = 1;

    int display_w = width;
    int display_h = height;
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

    // Interface scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    io.FontGlobalScale = scale;

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
            if (ImGui::BeginMainMenuBar()) {

                if (ImGui::BeginMenu("File")) {

                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImGui::Begin("Settings");

            ImGui::Text("Mandelbrot Explorer");

            if (ImGui::Button("Click me")) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("Clicks = %d", counter);

            ImGui::Text("Performance: %.3f ms/klatke (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("Resolution: %d x %d px", display_w, display_h);
            ImGui::End();
        }
        
        // Rendering ImGui
        ImGui::Render();

        // Screen cleaning (OpenGL)
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
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