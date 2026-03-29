#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

const unsigned int SCREEN_WIDTH{800};
const unsigned int SCREEN_HEIGHT{800};
const char *SCREEN_TITLE{"2026-03-29"};

const char *vertexShaderSource{R"(
#version 330 core
// the position variable has attribute position 0
layout (location = 0) in vec3 aPos;

// specify a color output to the vertex shader
out vec4 vertexColor;

void main() {
  gl_Position = vec4(aPos, 1.0);

  // set the output variable to a dark red color
  vertexColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)"};

const char *fragmentShaderSource{R"(
#version 330 core
out vec4 FragColor;

// we set this variable in the OpenGL code
uniform vec4 ourColor;

void main() {
  FragColor = ourColor;
}
)"};

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}

void framebufferSizecallback(GLFWwindow *window, int width, int height) {
  if (window == nullptr) {
    return;
  }
  glViewport(0, 0, width, height);
}

namespace utils {
void PrintGlfwGetTime() {
  auto currentTime{glfwGetTime()};
  std::cout << currentTime << '\n';
}
} // namespace utils

auto main() -> int {
  utils::PrintGlfwGetTime();
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto *window{glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE,
                                nullptr, nullptr)};
  if (window == nullptr) {
    std::cout << "Failed to create GLFW window" << '\n';
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizecallback);
  auto gladInitSuccess{gladLoadGL((GLADloadfunc)(glfwGetProcAddress)) == 0};

  if (gladInitSuccess) {
    std::cout << "Failed to initialize GLAD" << '\n';
    return -1;
  }

  utils::PrintGlfwGetTime();
  std::cout << "Vendor\t:" << glGetString(GL_VENDOR) << '\n';
  std::cout << "Renderer\t:" << glGetString(GL_RENDERER) << '\n';
  std::cout << "Version\t:" << glGetString(GL_VERSION) << '\n';

  /**
   * Declraing vertex shader
   * */
  auto vertexShader{glCreateShader(GL_VERTEX_SHADER)};
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  constexpr int INFO_LOG_SIZE{512};
  int success{};
  char info_log[INFO_LOG_SIZE];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if (success == 0) {
    glGetShaderInfoLog(vertexShader, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Compiling vertex shader failed" << info_log << '\n';
  }

  /**
   * Declraing fragment shader
   * */
  auto fragmentShader{glCreateShader(GL_FRAGMENT_SHADER)};
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (success == 0) {
    glGetShaderInfoLog(fragmentShader, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Compiling fragment shader failed" << info_log << '\n';
  }

  /**
   * INFO: Link shaders
   * */
  auto shaderProgram{glCreateProgram()};
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(shaderProgram, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Linking Failed" << info_log << '\n';
  }

  utils::PrintGlfwGetTime();

  /**
   * WARN: Must delete compiled shaders
   * */
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  /**
   * Define vertex data
   * */
  const float vertices[] = {
      0.5F,  -0.5F, 0.0F, // bottom right
      -0.5F, -0.5F, 0.0F, // bottom left
      0.0F,  0.5F,  0.0F, // top
  };

  unsigned int VBO{};
  unsigned int VAO{};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s)
  // and then configure vertex attributes
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  // glBindVertexArray(0); // WARN: unbind call
  glBindVertexArray(VAO);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // WARN: Polygon mode

  while (glfwWindowShouldClose(window) == 0) {
    // NOTE: input
    processInput(window);

    // NOTE: rendrer
    const float red_val{0.2F};
    const float green_val{0.3F};
    const float blue_val{0.3F};
    const float alpha_val{1.0F};

    // NOTE: clear the colorbuffer
    glClearColor(red_val, green_val, blue_val, alpha_val);
    glClear(GL_COLOR_BUFFER_BIT);

    // WARN: draw our first triangle, be sure to activate the shader
    glUseProgram(shaderProgram);

    // NOTE: swap buffers + poll IO events

    double timeValue{glfwGetTime()};
    const float divisor{2.0};
    const float sigma{0.5};
    float greenValue{static_cast<float>((cos(timeValue) / divisor) + sigma)};
    int vertexColorLocation{glGetUniformLocation(shaderProgram, "ourColor")};

    if (vertexColorLocation == -1) {
      std::cout << "ERROR: glGetUniformLocation did not find ourColor" << '\n';
    }

    glUniform4f(vertexColorLocation, 0.0F, greenValue, 0.0F, 1.0F);

    // NOTE: render the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // WARN: de-allocate all resources
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  glfwTerminate();
  return 0;
}
