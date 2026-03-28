#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

const std::string vertexShaderSource{R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main() {
  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)"};

const std::string fragmentShaderSource{R"(
#version 330 core
out vec4 FragColor;

void main() {
  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)"};

const unsigned int SCREEN_WIDTH{500};
const unsigned int SCREEN_HEIGHT{500};

auto main() -> int {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window{glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                      "Hello OpenGL World!", nullptr, nullptr)};
  if (window == nullptr) {
    std::cout << "Failed to create GLFW window" << '\n';
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  auto success{gladLoadGL((GLADloadfunc)(glfwGetProcAddress)) == 0};

  if (success) {
    std::cout << "Failed to initialize GLAD" << '\n';
    return -1;
  }

  std::cout << "Vendor\t:" << glGetString(GL_VENDOR) << '\n';
  std::cout << "Renderer\t:" << glGetString(GL_RENDERER) << '\n';
  std::cout << "Version\t:" << glGetString(GL_VERSION) << '\n';

  while (glfwWindowShouldClose(window) == 0) {
    processInput(window);
    const float red_val{0.2F};
    const float green_val{0.3F};
    const float blue_val{0.3F};
    const float alpha_val{1.0F};
    glClearColor(red_val, green_val, blue_val, alpha_val);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  if (window == nullptr) {
    return;
  }
  glViewport(0, 0, width, height);
}
