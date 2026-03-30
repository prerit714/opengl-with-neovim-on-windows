#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>

const unsigned int SCREEN_WIDTH{800};
const unsigned int SCREEN_HEIGHT{800};
const char *SCREEN_TITLE{"2026-03-29"};

const char *vertexShaderSource{R"(
#version 330 core
// the position variable has attribute position 0
layout (location = 0) in vec3 aPos;

// the color variable has attribute position 1
layout (location = 1) in vec3 aColor;

// specify a color output to the fragment shader
out vec3 ourColor;

void main() {
  gl_Position = vec4(aPos, 1.0);

  // set ourColor to the input color we got from vertex data
  ourColor = aColor;
}
)"};

const char *fragmentShaderSource{R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;

void main() {
  FragColor = vec4(ourColor, 1.0);
}
)"};

auto processInput(GLFWwindow *window) -> void {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}

auto framebufferSizecallback(GLFWwindow *window, int width, int height)
    -> void {
  if (window == nullptr) {
    return;
  }
  glViewport(0, 0, width, height);
}

auto main() -> int {
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

  /**
   * WARN: Must delete compiled shaders
   * */
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  /**
   * Define vertex data
   * */
  const float vertices[] = {
      // positions         // colors
      0.5F,  -0.5F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom right
      -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, // bottom leFt
      0.0F,  0.5F,  0.0F, 0.0F, 0.0F, 1.0F  // top
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

  // position attribute
  constexpr int STRIDE_NUMBER{6};
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE_NUMBER * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  // color attribute
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, STRIDE_NUMBER * sizeof(float),
      reinterpret_cast<void *>(static_cast<std::uintptr_t>(3 * sizeof(float))));
  glEnableVertexAttribArray(1);

  // glBindVertexArray(0); // WARN: unbind call
  glBindVertexArray(VAO);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // WARN: Polygon mode

  while (glfwWindowShouldClose(window) == 0) {
    // NOTE: input
    processInput(window);

    // NOTE: rendrer
    const auto red_val{0.2F};
    const auto green_val{0.3F};
    const auto blue_val{0.3F};
    const auto alpha_val{1.0F};

    // NOTE: clear the colorbuffer
    glClearColor(red_val, green_val, blue_val, alpha_val);
    glClear(GL_COLOR_BUFFER_BIT);

    // WARN: draw our first triangle, be sure to activate the shader
    glUseProgram(shaderProgram);

    // NOTE: render the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // NOTE: swap buffers + poll IO events
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
