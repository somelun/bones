#include <iostream>

#include "ufbx.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr int Width  = 600;
constexpr int Height = 600;

const char* vertexShaderSource = R"(
  #version 330 core
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 aColor;
  layout (location = 2) in vec2 aTex;

  out vec3 color;
  out vec2 texCoord;

  uniform float scale;

  void main() {
    gl_Position = vec4(aPos.x + aPos.x * scale, aPos.y + aPos.y * scale, aPos.z + aPos.z * scale, 1.0);
    color = aColor;
    texCoord = aTex;
  }
)";

const char* fragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;

  in vec3 color;
  in vec2 texCoord;

  uniform sampler2D tex0;

  void main() {
    FragColor = texture(tex0, texCoord);
  }
)";

struct Vertex {
  float position[3];
};

constexpr float s = 1.73205080757f;// sqrtf(3.0f);
GLfloat vertices[] = {
// position           | color           | UV
  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
   0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f
};

GLuint indices[] = {
  0, 2, 1,
  0, 3, 2
};

bool load_fbx(const char* path, std::vector<Vertex>& out_vertices) {
  ufbx_load_opts opts = {0};
  ufbx_error error;
  ufbx_scene *scene = ufbx_load_file(path, &opts, &error);

  if (!scene) {
    fprintf(stderr, "Failed to load: %s\n", error.description.data);
    return false;
  }

  for (size_t i = 0; i < scene->meshes.count; i++) {
    ufbx_mesh* mesh = scene->meshes[i];
    const ufbx_vec3* positions = mesh->vertex_position.values.data;

    for (size_t j = 0; j < mesh->num_vertices; j++) {
      Vertex v;
      ufbx_vec3 pos = positions[j];
      v.position[0] = (float)pos.x;
      v.position[1] = (float)pos.y;
      v.position[2] = (float)pos.z;
      out_vertices.push_back(v);
    }
  }

  ufbx_free_scene(scene);
  return true;
}

int main(int argc, char* argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_Window* window = SDL_CreateWindow(
    "bones",
    Width,
    Height,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
  );

  if (!window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) {
    std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_GL_MakeCurrent(window, context);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW!" << std::endl;
    return 1;
  }

  glViewport(0, 0, Width, Height);

  // std::vector<Vertex> vertices;
  // std::vector<GLuint> indices;
  // if (argc < 2) {
  //   test_object(vertices, indices);
  // } else {
  //   if (!load_fbx(argv[1], vertices)) {
  //     return 1;
  //   }
  // }

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLuint vao, vbo, ebo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLuint uniID = glGetUniformLocation(shaderProgram, "scale");

  // texture
  int tex_width, tex_heigth, tex_ch;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* texture_bytes = stbi_load(
      "../assets/texture128.png",
      &tex_width,
      &tex_heigth,
      &tex_ch,
      0);

  if (texture_bytes == NULL) {
    std::cerr << "Failed to load texture " << std::endl;
    return 1;
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      tex_width,
      tex_heigth,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      texture_bytes);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(texture_bytes);
  glBindTexture(GL_TEXTURE_2D, 0);

  GLuint tex0uni = glGetUniformLocation(shaderProgram, "tex0");
  glUseProgram(shaderProgram);
  glUniform1i(tex0uni, 0);

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;
        case SDL_EVENT_KEY_DOWN:
          if (event.key.key == SDLK_ESCAPE) {
            running = false;
          }
          break;
      }
    }

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniform1f(uniID, 0.5f);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteTextures(1, &texture);
  glDeleteProgram(shaderProgram);

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
