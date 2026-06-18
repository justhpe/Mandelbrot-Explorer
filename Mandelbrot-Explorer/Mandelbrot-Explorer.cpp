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


// Fragment Shader Code (FP64 - wysoka precyzja)
const char* fragmentShaderSourceFP64 = R"(
    #version 460 core
    out vec4 FragColor;

    uniform vec2 u_resolution;
    uniform double u_minR;
    uniform double u_maxR;
    uniform double u_minI;
    uniform double u_maxI;
    uniform int u_max_iterations;
    uniform int u_fractal_type; // 0 = Mandelbrot, 1 = Burning Ship, 2 = Julia
    uniform double u_julia_c_r;
    uniform double u_julia_c_i;
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

    // Paleta 1: Ogieñ (Fire)
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
        double cr_pixel = u_minR + (gl_FragCoord.x / u_resolution.x) * (u_maxR - u_minR);
        double cj_pixel = u_minI + (gl_FragCoord.y / u_resolution.y) * (u_maxI - u_minI);
        int max_iterations = u_max_iterations;

        double zr, zj, cr, cj;

        if (u_fractal_type == 2) {
            // Zbiór Julii
            zr = cr_pixel;
            zj = cj_pixel;
            cr = u_julia_c_r;
            cj = u_julia_c_i;
        } else {
            // Mandelbrot lub Burning Ship
            zr = 0.0;
            zj = 0.0;
            cr = cr_pixel;
            cj = cj_pixel;
        }

        int count = 0;

        while (zr * zr + zj * zj <= 256.0 && count < max_iterations) {
            double temp = zr * zr - zj * zj + cr;
            
            if (u_fractal_type == 1) { // Burning Ship
                zj = -abs(2.0 * zr * zj) + cj;
            } else { // Mandelbrot & Julia
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

// Fragment Shader Code (FP32 - szybki, ale mniej precyzyjny)
const char* fragmentShaderSourceFP32 = R"(
    #version 460 core
    out vec4 FragColor;

    uniform vec2 u_resolution;
    uniform double u_minR;
    uniform double u_maxR;
    uniform double u_minI;
    uniform double u_maxI;
    uniform int u_max_iterations;
    uniform int u_fractal_type;
    uniform double u_julia_c_r;
    uniform double u_julia_c_i;
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

    // Paleta 1: Ogieñ (Fire)
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

        float cr_pixel = float(cr_d);
        float cj_pixel = float(cj_d);
        int max_iterations = u_max_iterations;

        float zr, zj, cr, cj;

        if (u_fractal_type == 2) {
            zr = cr_pixel;
            zj = cj_pixel;
            cr = float(u_julia_c_r);
            cj = float(u_julia_c_i);
        } else {
            zr = 0.0;
            zj = 0.0;
            cr = cr_pixel;
            cj = cj_pixel;
        }

        int count = 0;

        while (zr * zr + zj * zj <= 256.0 && count < max_iterations) {
            float temp = zr * zr - zj * zj + cr;
            
            if (u_fractal_type == 1) { // Burning Ship
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

int main()
{
    // Config
    int width = 1280;
    int height = 720;
    float scale = 1;
    bool useFP64 = false; // Domyœlnie na false
    int fractalMode = 0;
    const char* fractalModes[] = { "Mandelbrot set", "Burning Ship", "Julia set" };

    // Sta³e dla Zbioru Julii
    double julia_c_r = -0.7;
    double julia_c_i = 0.27015;

    int current_palette = 0;
    const char* palettes[] = { "Wikipedia (Klasyczna)", "Ogien (Fire)", "Lod (Ice)", "Neon" };
    bool showSettings = true;

    bool isFullscreen = false;
    int windowedX = 100;
    int windowedY = 100;
    int windowedWidth = width;
    int windowedHeight = height;

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

    int max_iterations = 150;
    bool auto_iterations = true;

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

    // UI scaling
    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    scale = xscale;

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

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);

    io.Fonts->Clear();
    ImFont* font = nullptr;

    font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f * scale);

    if (font == nullptr) {
        font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f * scale);
    }

    if (font == nullptr) {
        ImFontConfig config;
        config.SizePixels = 13.0f * scale;
        io.Fonts->AddFontDefault(&config);
    }

    io.FontGlobalScale = 1.0f;

    // Connecting ImGui to GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    auto toggleFullscreen = [&]() {
        isFullscreen = !isFullscreen;
        if (isFullscreen) {
            glfwGetWindowPos(window, &windowedX, &windowedY);
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
        }
        glfwSetWindowAspectRatio(window, 16, 9);
        };

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Obliczanie poziomu powiêkszenia (zoom) i szerokoœci
        double currentWidth = maxR - minR;
        double zoomLevel = 3.0 / currentWidth;

        // Auto Iteracje
        if (auto_iterations) {
            int dynamic_iter = 100 + (int)(log10(std::max(1.0, zoomLevel)) * 120.0);
            max_iterations = std::min(dynamic_iter, 3000);
        }

        {
            if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
                toggleFullscreen();
            }

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem(useFP64 ? "Use FP32" : "Use FP64")) {
                        useFP64 = !useFP64;
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Window")) {
                    if (ImGui::MenuItem("Settings")) {
                        showSettings = !showSettings;
                    }
                    if (ImGui::MenuItem("Toggle Fullscreen", "F11")) {
                        toggleFullscreen();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            if (showSettings) {
                ImGui::Begin("Settings");

                ImGui::Text("Mandelbrot Explorer");

                if (ImGui::Button("Reset View")) {
                    targetMinR = -2.0;
                    targetMaxR = 1.0;
                    targetMinI = -1.2;
                    targetMaxI = 1.2;
                }

                ImGui::Separator();
                ImGui::Text("Zoom stats:");

                if (zoomLevel > 10000.0) {
                    ImGui::Text("Zoom: %.2e x", zoomLevel);
                }
                else {
                    ImGui::Text("Zoom: %.1f x", zoomLevel);
                }

                ImGui::Text("Width (Delta R): %.2e", currentWidth);

                ImGui::Separator();
                ImGui::Combo("Color set", &current_palette, palettes, IM_ARRAYSIZE(palettes));
                ImGui::Combo("Fractal Type", &fractalMode, fractalModes, IM_ARRAYSIZE(fractalModes));

                if (fractalMode == 2) {
                    ImGui::Text("Opcje Zbioru Julii:");
                    ImGui::InputDouble("Re(c)", &julia_c_r, 0.001, 0.01, "%.6f");
                    ImGui::InputDouble("Im(c)", &julia_c_i, 0.001, 0.01, "%.6f");
                }

                ImGui::Separator();
                ImGui::Checkbox("Auto-Adjust Iterations", &auto_iterations);

                if (!auto_iterations) {
                    ImGui::SliderInt("Max Iteration", &max_iterations, 10, 2000);
                }
                else {
                    ImGui::Text("Obecne Iteracje: %d", max_iterations);
                }

                ImGui::SliderFloat("Zoom speed", &zoomSpeed, 0.01, 1);

                ImGui::Separator();
                ImGui::Checkbox("Use FP64?", &useFP64);
                ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::Text("Resolution: %d x %d px", width, height);
                ImGui::End();
            }
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
                float dx = dragCurrent.x - dragStart.x;
                float signX = (dx >= 0.0f) ? 1.0f : -1.0f;
                float absDx = fabs(dx);
                float absDy = absDx * (9.0f / 16.0f);
                float signY = (dragCurrent.y >= dragStart.y) ? 1.0f : -1.0f;

                ImVec2 lockedEnd(dragStart.x + absDx * signX, dragStart.y + absDy * signY);
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                drawList->AddRect(dragStart, lockedEnd, IM_COL32(255, 215, 0, 255), 0.0f, 0, 2.0f);
            }

            if (isDragging && ImGui::IsMouseReleased(0)) {
                ImVec2 dragEnd = ImGui::GetMousePos();
                isDragging = false;
                float dx = dragEnd.x - dragStart.x;

                if (fabs(dx) > 10.0f) {
                    float signX = (dx >= 0.0f) ? 1.0f : -1.0f;
                    float absDx = fabs(dx);
                    float absDy = absDx * (9.0f / 16.0f);
                    float signY = (dragEnd.y >= dragStart.y) ? 1.0f : -1.0f;

                    ImVec2 lockedEnd(dragStart.x + absDx * signX, dragStart.y + absDy * signY);
                    double x1 = dragStart.x;
                    double y1 = height - dragStart.y;
                    double x2 = lockedEnd.x;
                    double y2 = height - lockedEnd.y;

                    double screenMinX = std::min(x1, x2);
                    double screenMaxX = std::max(x1, x2);
                    double screenMinY = std::min(y1, y2);
                    double screenMaxY = std::max(y1, y2);

                    double currentRangeR = maxR - minR;
                    double currentRangeI = maxI - minI;

                    double newMinR = minR + (screenMinX / width) * currentRangeR;
                    double newMaxR = minR + (screenMaxX / width) * currentRangeR;
                    double newMinI = minI + (screenMinY / height) * currentRangeI;
                    double newMaxI = minI + (screenMaxY / height) * currentRangeI;

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

                double ratioX = mouseOpenGlX / width;
                double ratioY = mouseOpenGlY / height;

                targetMinR = mouseR - ratioX * newRangeR;
                targetMaxR = mouseR + (1.0 - ratioX) * newRangeR;
                targetMinI = mouseI - ratioY * newRangeI;
                targetMaxI = mouseI + (1.0 - ratioY) * newRangeI;
            }
        }

        ImGui::Render();

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram = useFP64 ? shaderProgramFP64 : shaderProgramFP32;
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

        glUniform1i(glGetUniformLocation(shaderProgram, "u_fractal_type"), fractalMode);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_julia_c_r"), julia_c_r);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_julia_c_i"), julia_c_i);
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

    return 0;
}