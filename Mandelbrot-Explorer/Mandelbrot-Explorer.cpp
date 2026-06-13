#include <glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

#include <glfw3.h>

// Vertex Shader Code
const char* vertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";


// Fragment Shader Code (FP64)
const char* fragmentShaderSourceFP64 = R"(
    // Fragment Shader Code (FP64)
    #version 460 core
    out vec4 FragColor;

    uniform vec2 u_resolution;
    uniform double u_minR;
    uniform double u_maxR;
    uniform double u_minI;
    uniform double u_maxI;
    uniform int u_max_iterations;
    uniform bool u_burning_ship;
    uniform int u_color_set; 

    // Paleta 0: Wikipedia
    vec3 getWiki(float t) {
        t = fract(t);
        if (t < 0.16)      return mix(vec3(0.0, 0.0, 0.1), vec3(0.1, 0.3, 0.8), t / 0.16);
        else if (t < 0.42) return mix(vec3(0.1, 0.3, 0.8), vec3(0.9, 0.95, 1.0), (t - 0.16) / 0.26);
        else if (t < 0.64) return mix(vec3(0.9, 0.95, 1.0), vec3(1.0, 0.6, 0.0), (t - 0.42) / 0.22);
        else if (t < 0.85) return mix(vec3(1.0, 0.6, 0.0), vec3(0.0, 0.0, 0.0), (t - 0.64) / 0.21);
        else               return mix(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.1), (t - 0.85) / 0.15);
    }

    // Paleta 1: Ogień (Fire)
    vec3 getFire(float t) {
        t = fract(t);
        if (t < 0.25)      return mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), t / 0.25);
        else if (t < 0.50) return mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 0.5, 0.0), (t - 0.25) / 0.25);
        else if (t < 0.75) return mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.0), (t - 0.50) / 0.25);
        else               return mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), (t - 0.75) / 0.25);
    }

    // Paleta 2: Lód (Ice)
    vec3 getIce(float t) {
        t = fract(t);
        if (t < 0.33)      return mix(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.5), t / 0.33);
        else if (t < 0.66) return mix(vec3(0.0, 0.0, 0.5), vec3(0.0, 0.8, 1.0), (t - 0.33) / 0.33);
        else               return mix(vec3(0.0, 0.8, 1.0), vec3(1.0, 1.0, 1.0), (t - 0.66) / 0.34);
    }

    // Paleta 3: Neon
    vec3 getNeon(float t) {
        t = fract(t);
        if (t < 0.33)      return mix(vec3(0.0, 0.0, 0.0), vec3(0.3, 0.0, 0.5), t / 0.33);
        else if (t < 0.66) return mix(vec3(0.3, 0.0, 0.5), vec3(1.0, 0.0, 0.8), (t - 0.33) / 0.33);
        else               return mix(vec3(1.0, 0.0, 0.8), vec3(0.0, 1.0, 1.0), (t - 0.66) / 0.34);
    }

    vec3 getColor(float t) {
        if (u_color_set == 1) return getFire(t);
        if (u_color_set == 2) return getIce(t);
        if (u_color_set == 3) return getNeon(t);
        return getWiki(t); 
    }

    void main() {
        double cr = u_minR + (gl_FragCoord.x / u_resolution.x) * (u_maxR - u_minR);
        double cj = u_minI + (gl_FragCoord.y / u_resolution.y) * (u_maxI - u_minI);
        int max_iterations = u_max_iterations;

        double zr = 0.0;
        double zj = 0.0;
        int count = 0;

        while (zr * zr + zj * zj <= 256.0 && count < max_iterations) {
            double temp = zr * zr - zj * zj + cr;
            
            if (u_burning_ship) {
                zj = -abs(2.0 * zr * zj) + cj;
            } else {
                zj = 2.0 * zr * zj + cj;
            }
            
            zr = temp;
            count++;
        }

        if (count == max_iterations) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            float f_z_sq = float(zr * zr + zj * zj);
            float log_z = log(f_z_sq) / 2.0;
            float nu = log(log_z / log(2.0)) / log(2.0);
            
            float smooth_iter = float(count) + 1.0 - nu;

            float color_index = smooth_iter * 0.05;
            FragColor = vec4(getColor(color_index), 1.0);
        }
    }
)";

// Fragment Shader Code (FP32)
const char* fragmentShaderSourceFP32 = R"(
    // Fragment Shader Code (FP32)
    #version 460 core
    out vec4 FragColor;

    uniform vec2 u_resolution;
    uniform double u_minR;
    uniform double u_maxR;
    uniform double u_minI;
    uniform double u_maxI;
    uniform int u_max_iterations;
    uniform bool u_burning_ship;
    uniform int u_color_set;

    // Paleta 0: Wikipedia
    vec3 getWiki(float t) {
        t = fract(t);
        if (t < 0.16)      return mix(vec3(0.0, 0.0, 0.1), vec3(0.1, 0.3, 0.8), t / 0.16);
        else if (t < 0.42) return mix(vec3(0.1, 0.3, 0.8), vec3(0.9, 0.95, 1.0), (t - 0.16) / 0.26);
        else if (t < 0.64) return mix(vec3(0.9, 0.95, 1.0), vec3(1.0, 0.6, 0.0), (t - 0.42) / 0.22);
        else if (t < 0.85) return mix(vec3(1.0, 0.6, 0.0), vec3(0.0, 0.0, 0.0), (t - 0.64) / 0.21);
        else               return mix(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.1), (t - 0.85) / 0.15);
    }

    // Paleta 1: Ogień (Fire)
    vec3 getFire(float t) {
        t = fract(t);
        if (t < 0.25)      return mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), t / 0.25);
        else if (t < 0.50) return mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 0.5, 0.0), (t - 0.25) / 0.25);
        else if (t < 0.75) return mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.0), (t - 0.50) / 0.25);
        else               return mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), (t - 0.75) / 0.25);
    }

    // Paleta 2: Lód (Ice)
    vec3 getIce(float t) {
        t = fract(t);
        if (t < 0.33)      return mix(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.5), t / 0.33);
        else if (t < 0.66) return mix(vec3(0.0, 0.0, 0.5), vec3(0.0, 0.8, 1.0), (t - 0.33) / 0.33);
        else               return mix(vec3(0.0, 0.8, 1.0), vec3(1.0, 1.0, 1.0), (t - 0.66) / 0.34);
    }

    // Paleta 3: Neon
    vec3 getNeon(float t) {
        t = fract(t);
        if (t < 0.33)      return mix(vec3(0.0, 0.0, 0.0), vec3(0.3, 0.0, 0.5), t / 0.33);
        else if (t < 0.66) return mix(vec3(0.3, 0.0, 0.5), vec3(1.0, 0.0, 0.8), (t - 0.33) / 0.33);
        else               return mix(vec3(1.0, 0.0, 0.8), vec3(0.0, 1.0, 1.0), (t - 0.66) / 0.34);
    }

    vec3 getColor(float t) {
        if (u_color_set == 1) return getFire(t);
        if (u_color_set == 2) return getIce(t);
        if (u_color_set == 3) return getNeon(t);
        return getWiki(t); 
    }

    void main() {
        double cr_d = u_minR + (gl_FragCoord.x / u_resolution.x) * (u_maxR - u_minR);
        double cj_d = u_minI + (gl_FragCoord.y / u_resolution.y) * (u_maxI - u_minI);

        float cr = float(cr_d);
        float cj = float(cj_d);
        int max_iterations = u_max_iterations;

        float zr = 0.0; 
        float zj = 0.0; 
        int count = 0;

        while (zr * zr + zj * zj <= 256.0 && count < max_iterations) {
            float temp = zr * zr - zj * zj + cr;
            
            if (u_burning_ship) {
                zj = -abs(2.0 * zr * zj) + cj;
            } else {
                zj = 2.0 * zr * zj + cj;
            }
            
            zr = temp;
            count++;
        }

        if (count == max_iterations) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
        } else {
            float z_sq = zr * zr + zj * zj;
            float log_z = log(z_sq) / 2.0;
            float nu = log(log_z / log(2.0)) / log(2.0);
            
            float smooth_iter = float(count) + 1.0 - nu;

            float color_index = smooth_iter * 0.05;
            FragColor = vec4(getColor(color_index), 1.0);
        }
    }
)";


// Shader compilation
GLuint compileShaders(const char* fragmentShaderSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


// Unused, as all compute are performed on GPU
// It can be safely removed
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
    int width = 1280;
    int height = 720;
    float scale = 1;
    bool useFP64 = false;
    bool showBurningShip = false;

    int current_palette = 0;
    const char* palettes[] = { "Wikipedia (Klasyczna)", "Ogien (Fire)", "Lod (Ice)", "Neon" };

    double minR = -2.0;
    double maxR = 1.0;
    double minI = -1.2;
    double maxI = 1.2;

    // For smooth zoom
    double targetMinR = minR;
    double targetMaxR = maxR;
    double targetMinI = minI;
    double targetMaxI = maxI;

    float zoomSpeed = 0.1;

    int max_iterations = 100;

    // GLFW Init
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // GLAD Init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

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

    // Compiling the shader program
    GLuint shaderProgramFP32 = compileShaders(fragmentShaderSourceFP32);
    GLuint shaderProgramFP64 = compileShaders(fragmentShaderSourceFP64);
    GLuint shaderProgram = shaderProgramFP32; // default shader program

    // Two triangles forming a rectangle
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

                    ImGui::Separator();

                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImGui::Begin("Settings");

            ImGui::Text("Mandelbrot Explorer");

            if (ImGui::Button("Reset View")) {
                targetMinR = -2.0; 
                targetMaxR = 1.0; 
                targetMinI = -1.2; 
                targetMaxI = 1.2;
            }

            // Current width in complex numbers
            double currentWidth = maxR - minR;
            double zoomLevel = 3.0 / currentWidth;

            ImGui::Separator();
            ImGui::Text("Zoom stats:");

            if (zoomLevel > 10000.0) {
                ImGui::Text("Zoom: %.2e x", zoomLevel);
            }
            else {
                ImGui::Text("Zoom: %.1f x", zoomLevel);
            }

            ImGui::Text("Szerokosc (Delta R): %.2e", currentWidth);

            if (ImGui::Button("Click me")) {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("Clicks = %d", counter);

            ImGui::Separator();
            ImGui::Combo("Kolorystyka", &current_palette, palettes, IM_ARRAYSIZE(palettes));

            ImGui::Checkbox("Use FP64?", &useFP64);
            ImGui::Checkbox("Burning Ship", &showBurningShip);

            ImGui::SliderInt("Max Iteration", &max_iterations, 10, 1000);

            ImGui::SliderFloat("Zoom speed", &zoomSpeed, 0.01, 1);

            ImGui::Separator();
            ImGui::Text("Performance: %.3f ms/klatke (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("Resolution: %d x %d px", width, height);
            ImGui::End();
        }

        static ImVec2 dragStart(0, 0);
        static bool isDragging = false;

        if (!ImGui::GetIO().WantCaptureMouse) {

            if (ImGui::IsMouseClicked(0)) {
                dragStart = ImGui::GetMousePos();
                isDragging = true;
            }

            if (isDragging && ImGui::IsMouseDragging(0)) {
                ImVec2 dragCurrent = ImGui::GetMousePos();

                // Calculate current offset
                float dx = dragCurrent.x - dragStart.x;
                float signX = (dx >= 0.0f) ? 1.0f : -1.0f;

                // Force 16:9 aspect ratio based on width
                float absDx = fabs(dx);
                float absDy = absDx * (9.0f / 16.0f);

                // Get vertical drag direction
                float signY = (dragCurrent.y >= dragStart.y) ? 1.0f : -1.0f;

                // Calculate 16:9 rectangle end point
                ImVec2 lockedEnd(dragStart.x + absDx * signX, dragStart.y + absDy * signY);

                // Draw 16:9 gold frame
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                drawList->AddRect(dragStart, lockedEnd, IM_COL32(255, 215, 0, 255), 0.0f, 0, 2.0f); // Gold frame
            }

            if (isDragging && ImGui::IsMouseReleased(0)) {
                ImVec2 dragEnd = ImGui::GetMousePos();
                isDragging = false;

                float dx = dragEnd.x - dragStart.x;

                // Min 10px threshold to prevent accidental clicks
                if (fabs(dx) > 10.0f) {
                    float signX = (dx >= 0.0f) ? 1.0f : -1.0f;
                    float absDx = fabs(dx);
                    float absDy = absDx * (9.0f / 16.0f);
                    float signY = (dragEnd.y >= dragStart.y) ? 1.0f : -1.0f;

                    // Reconstruct 16:9 end point
                    ImVec2 lockedEnd(dragStart.x + absDx * signX, dragStart.y + absDy * signY);

                    // Map to OpenGL coordinates (Y-up)
                    double x1 = dragStart.x;
                    double y1 = height - dragStart.y;
                    double x2 = lockedEnd.x;
                    double y2 = height - lockedEnd.y;

                    // Sort min/max
                    double screenMinX = std::min(x1, x2);
                    double screenMaxX = std::max(x1, x2);
                    double screenMinY = std::min(y1, y2);
                    double screenMaxY = std::max(y1, y2);

                    // Convert pixels to complex plane
                    double currentRangeR = maxR - minR;
                    double currentRangeI = maxI - minI;

                    double newMinR = minR + (screenMinX / width) * currentRangeR;
                    double newMaxR = minR + (screenMaxX / width) * currentRangeR;
                    double newMinI = minI + (screenMinY / height) * currentRangeI;
                    double newMaxI = minI + (screenMaxY / height) * currentRangeI;

                    // Update fractal world bounds
                    targetMinR = newMinR;
                    targetMaxR = newMaxR;
                    targetMinI = newMinI;
                    targetMaxI = newMaxI;
                }
            }
        }
        else {
            isDragging = false;
        }

        // Scroll zoom
        if (!ImGui::GetIO().WantCaptureMouse) {
            float wheel = ImGui::GetIO().MouseWheel;

            if (wheel != 0.0f) {
                ImVec2 mousePos = ImGui::GetMousePos();

                // Mapping the current mouse position to the complex plane
                double mouseOpenGlX = mousePos.x;
                double mouseOpenGlY = height - mousePos.y;

                double currentRangeR = maxR - minR;
                double currentRangeI = maxI - minI;

                double mouseR = minR + (mouseOpenGlX / width) * currentRangeR;
                double mouseI = minI + (mouseOpenGlY / height) * currentRangeI;

                double zoomFactor = 1.0;
                if (wheel > 0.0f) {
                    zoomFactor = 0.85; // zoom in
                }
                else {
                    zoomFactor = 1.15; // zoom out
                }

                double newRangeR = currentRangeR * zoomFactor;
                double newRangeI = currentRangeI * zoomFactor;

                // Cursor is an anchor
                double ratioX = mouseOpenGlX / width;
                double ratioY = mouseOpenGlY / height;

                targetMinR = mouseR - ratioX * newRangeR;
                targetMaxR = mouseR + (1.0 - ratioX) * newRangeR;
                targetMinI = mouseI - ratioY * newRangeI;
                targetMaxI = mouseI + (1.0 - ratioY) * newRangeI;
            }
        }

        // Rendering ImGui
        ImGui::Render();


        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram = useFP64 ? shaderProgramFP64 : shaderProgramFP32;

        // GPU program starts here
        glUseProgram(shaderProgram);

		// Linear Interpolation for smooth zooming
        minR += (targetMinR - minR) * zoomSpeed;
        maxR += (targetMaxR - maxR) * zoomSpeed;
        minI += (targetMinI - minI) * zoomSpeed;
        maxI += (targetMaxI - maxI) * zoomSpeed;

        // Sending variables to Shader code
        glUniform2f(glGetUniformLocation(shaderProgram, "u_resolution"), (float)width, (float)height);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_max_iterations"), max_iterations);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_minR"), minR);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_maxR"), maxR);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_minI"), minI);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_maxI"), maxI);

        glUniform1i(glGetUniformLocation(shaderProgram, "u_burning_ship"), showBurningShip);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_color_set"), current_palette);

        // GPU draws rectangle filled with shader
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Drawing the interface on the screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Displaying a frame on the screen
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgramFP32);
    glDeleteProgram(shaderProgramFP64);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

}