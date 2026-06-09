#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <cstdint>

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

    double minR = -2.0;
    double maxR = 1.0;
    double minI = -1.2;
    double maxI = 1.2;

    int max_iterations = 100;
    std::vector<uint32_t> buffer(width * height);

    // GLFW Init
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // Creating a system window
    GLFWwindow* window = glfwCreateWindow(width, height, "Mandelbrot Explorer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowAspectRatio(window, 16, 9); // keep aspect ratio
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

    // Empty OpenGL Texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
            ImGui::Text("Resolution: %d x %d px", width, height);
            ImGui::End();
        }
        
        // Rendering ImGui
        ImGui::Render();

                
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Mapowanie pixela na plaszczyzne zespolona
                double cr = minR + (double)x / width * (maxR - minR);
                double ci = minI + (double)y / height * (maxI - minI);

                int iterations = mandelbrot(cr, ci, max_iterations);

                // Kolorowanie i zapis do bufora (ARGB)
                if (iterations == max_iterations) {
                    buffer[y * width + x] = 0xFF000000; // Black (alfa, B, G, R)
                }
                else {
                    uint8_t r = iterations * 2;
                    uint8_t g = iterations * 5;
                    uint8_t b = iterations;
                    buffer[y * width + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                }
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());


        // Screen cleaning (OpenGL)
        int display_w;
        int display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dynamic resolution change
        if (display_h != height || display_w != width) {
            height = display_h;
            width = display_w;
            buffer.resize(width * height);
        }

        // Enabling 2D texturing
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Rectangle over the entire window in OpenGL coordinates
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        // Drawing the interface on the screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Displaying a frame on the screen
        glfwSwapBuffers(window);
    }

    glDeleteTextures(1, &textureID);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    

}