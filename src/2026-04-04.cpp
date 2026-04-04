#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>

#include <stb_image.h>

constexpr int INFO_LOG_SIZE{512};
constexpr unsigned int SCREEN_WIDTH{800};
constexpr unsigned int SCREEN_HEIGHT{800};
constexpr std::string SCREEN_TITLE{"2026-04-04"};

const auto SHADER_DIRECTORY{std::filesystem::path{"shaders"}};
const auto ASSETS_DIRECTORY{std::filesystem::path("assets")};
const auto CONTAINER_ASSET_PATH{ASSETS_DIRECTORY / "container.jpg"};
const auto VERTEX_SHADER_PATH{SHADER_DIRECTORY / "basic.vert.glsl"};
const auto FRAGMENT_SHADER_PATH{SHADER_DIRECTORY / "basic.frag.glsl"};

auto loadShaderSource(const std::filesystem::path &shaderPath) -> std::string {
  const std::ifstream shaderFile{shaderPath};
  if (!shaderFile.is_open()) {
    std::ostringstream error{};
    error << "Failed to open shader file: " << shaderPath.string();
    throw std::runtime_error{error.str()};
  }

  std::ostringstream buffer{};
  buffer << shaderFile.rdbuf();
  return buffer.str();
}

auto compileShader(const GLenum &shaderType,
                   const std::filesystem::path &shaderPath) -> unsigned int {
  const auto shaderSource{loadShaderSource(shaderPath)};
  const auto *shaderSourcePtr{shaderSource.c_str()};

  auto shader{glCreateShader(shaderType)};
  glShaderSource(shader, 1, &shaderSourcePtr, nullptr);
  glCompileShader(shader);

  int success{};
  std::array<char, INFO_LOG_SIZE> infoLog{};
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (success == 0) {
    glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, infoLog.data());
    std::ostringstream error{};
    error << "Failed to compile shader '" << shaderPath.string()
          << "': " << infoLog.data();
    throw std::runtime_error{error.str()};
  }

  return shader;
}

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
  try {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto *window{glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                  SCREEN_TITLE.c_str(), nullptr, nullptr)};
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

    std::cout << "Vendor\t:" << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer\t:" << glGetString(GL_RENDERER) << '\n';
    std::cout << "Version\t:" << glGetString(GL_VERSION) << '\n';

    auto vertexShader{compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_PATH)};
    auto fragmentShader{
        compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_PATH)};

    /**
     * INFO: Link shaders
     * */
    auto shaderProgram{glCreateProgram()};
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

#ifdef NO_APP_POLYGON_MODE
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif

    int success{};
    std::array<char, INFO_LOG_SIZE> infoLog{};

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(shaderProgram, INFO_LOG_SIZE, nullptr,
                          infoLog.data());
      std::cout << "Error: Linking Failed " << infoLog.data() << '\n';
    }

    /**
     * WARN: Must delete compiled shaders
     * */
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /**
     * Define vertex2 data
     * */
    const std::array<float, 32> vertices{
        // positions          // colors           // texture coords
        0.5F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F, // top right
        0.5F,  -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, // bottom right
        -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom leFt
        -0.5F, 0.5F,  0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F  // top leFt
    };

    const std::array<unsigned int, 6> indicies{
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VBO{};
    unsigned int VAO{};
    unsigned int EBO{};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indicies.size() * sizeof(unsigned int), indicies.data(),
                 GL_STATIC_DRAW);

    constexpr int STRIDE_NUMBER{8};

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          STRIDE_NUMBER * sizeof(float),
                          static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          STRIDE_NUMBER * sizeof(float),
                          reinterpret_cast<void *>(
                              static_cast<std::uintptr_t>(3 * sizeof(float))));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          STRIDE_NUMBER * sizeof(float),
                          reinterpret_cast<void *>(
                              static_cast<std::uintptr_t>(3 * sizeof(float))));
    glEnableVertexAttribArray(2);

    // load and create a texture
    // -------------------------
    int width{};
    int height{};
    int nrChannels{};

    const auto texturePath{CONTAINER_ASSET_PATH.string()};
    auto *const data{
        stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0)};

    if (data == nullptr) {
      auto const *message{"Unable to load image"};
      std::cout << message << '\n';
      return 1;
    }

    // TODO: Use data

    unsigned int texture{};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // setting the texture wrapping to GL_REPEAT
                                // (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_REPEAT); // setting the texture wrapping to GL_REPEAT
                                // (default wrapping method)
    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // free the image data
    stbi_image_free(data);

#ifdef NO_UNBIND_CALL
    {
      glBindVertexArray(0);
    }
#endif

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
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT,
                     indicies.data());

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
  } catch (const std::exception &error) {
    std::cout << error.what() << '\n';
    glfwTerminate();
    return -1;
  }
}
