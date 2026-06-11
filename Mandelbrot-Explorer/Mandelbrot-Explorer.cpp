#include <glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <cstdint>

#include <glfw3.h>

// Vertex Shader Code
const char* vertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";


// Fragment Shader Code
const char* fragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;

    uniform vec2 u_resolution;
    uniform double u_minR;
    uniform double u_maxR;
    uniform double u_minI;
    uniform double u_maxI;
    uniform int u_max_iterations;

    void main() {
        // Mapping pixel pos to the complex plane
        double cr = u_minR + (gl_FragCoord.x / u_resolution.x) * (u_maxR - u_minR);
        double cj = u_minI + (gl_FragCoord.y / u_resolution.y) * (u_maxI - u_minI);
        int max_iterations = u_max_iterations;


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


        if (count == max_iterations) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black (R, G, B, alfa)
        } else {
            float r = float(count * 2) / 255.0;
            float g = float(count * 5) / 255.0;
            float b = float(count) / 255.0;
            FragColor = vec4(r, g, b, 1.0);
        }
    }
)";



// Shader compilation
GLuint compileShaders() {
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

    double minR = -2.0;
    double maxR = 1.0;
    double minI = -1.2;
    double maxI = 1.2;

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
    GLuint shaderProgram = compileShaders();

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


        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // GPU program starts here
        glUseProgram(shaderProgram);

        // Sending variables to Shader code
        glUniform2f(glGetUniformLocation(shaderProgram, "u_resolution"), (float)width, (float)height);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_max_iterations"), max_iterations);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_minR"), minR);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_maxR"), maxR);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_minI"), minI);
        glUniform1d(glGetUniformLocation(shaderProgram, "u_maxI"), maxI);

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
    glDeleteProgram(shaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    

}