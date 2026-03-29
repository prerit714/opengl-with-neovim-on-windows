#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

const char *vertex_shader_source{R"(
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

const char *fragment_shader_source{R"(
#version 330 core
out vec4 FragColor;

// the input variable from the vertex shader
// with the same name and same dimentions
in vec4 vertexColor;

void main() {
  FragColor = vertexColor;
}
)"};

const unsigned int SCREEN_WIDTH{800};
const unsigned int SCREEN_HEIGHT{800};
const char *SCREEN_TITLE{"2026-03-29"};

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
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  auto gladInitSuccess{gladLoadGL((GLADloadfunc)(glfwGetProcAddress)) == 0};

  if (gladInitSuccess) {
    std::cout << "Failed to initialize GLAD" << '\n';
    return -1;
  }

  std::cout << "Vendor\t:" << glGetString(GL_VENDOR) << '\n';
  std::cout << "Renderer\t:" << glGetString(GL_RENDERER) << '\n';
  std::cout << "Version\t:" << glGetString(GL_VERSION) << '\n';

  /**
   * Declraing vertex shader
   * */
  auto vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
  glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
  glCompileShader(vertex_shader);

  constexpr int INFO_LOG_SIZE{512};
  int success{};
  char info_log[INFO_LOG_SIZE];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

  if (success == 0) {
    glGetShaderInfoLog(vertex_shader, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Compiling vertex shader failed" << info_log << '\n';
  }

  /**
   * Declraing fragment shader
   * */
  auto fragment_shader{glCreateShader(GL_FRAGMENT_SHADER)};
  glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

  if (success == 0) {
    glGetShaderInfoLog(fragment_shader, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Compiling fragment shader failed" << info_log << '\n';
  }

  /**
   * INFO: Link shaders
   * */
  auto shader_program{glCreateProgram()};
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(shader_program, INFO_LOG_SIZE, nullptr, info_log);
    std::cout << "Error: Linking Failed" << info_log << '\n';
  }

  /**
   * WARN: Must delete compiled shaders
   * */
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  /**
   * Define vertex data
   * */

  const float vertices[] = {
      0.5F,  0.5F,  0.0F, // top right
      0.5F,  -0.5F, 0.0F, // bottom right
      -0.5F, -0.5F, 0.0F, // bottom left
      -0.5F, 0.5F,  0.0F, // top left
  };

  constexpr int NUMBER_OF_INDICES{6};

  // NOTE: we start from 0!
  unsigned int indices[NUMBER_OF_INDICES] = {
      0, 1, 3, // first triangle
      1, 2, 3, // second triangle
  };

  unsigned int VBO{};
  unsigned int VAO{};
  unsigned int EBO{};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // NOTE: bind the VAO first, then bind and set VBOs
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  // WARN: note that this is allowed, the call toglVertexAttribPointer
  // registered VBO as the vertex attribute's bound vertex buffer object so
  // afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // WARN: remember: do NOT unbind the EBO while a VAO is active as the bound
  // element buffer object IS stored in the VAO; keep the EBO bound.
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // WARN: This should be always be
  // a comment

  // WARN: You can unbind the VAO afterwards so other VAO calls won't
  // accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call toglBindVertexArray anyways so we generally don't
  // unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  // WARN: uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  while (glfwWindowShouldClose(window) == 0) {
    // input
    processInput(window);

    // rendrer
    const float red_val{0.2F};
    const float green_val{0.3F};
    const float blue_val{0.3F};
    const float alpha_val{1.0F};
    glClearColor(red_val, green_val, blue_val, alpha_val);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    glUseProgram(shader_program);
    glBindVertexArray(
        VAO); // seeing as we only have a single VAO there's no need to bind it
              // every time, but we'll do so to keep things a bit more organized
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, NUMBER_OF_INDICES, GL_UNSIGNED_INT, nullptr);
    // glBindVertexArray(0); // INFO: No need to unbind it everytime

    // swap buffers + poll IO
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // de-allocate all resources
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader_program);

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
