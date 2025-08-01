#include <iostream>

#include "ufbx.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

const char* vertexShaderSource = R"(
  #version 330 core
  layout (location = 0) in vec3 aPos;
  void main() {
    gl_Position = vec4(aPos, 1.0);
  }
)";

const char* fragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;
  void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  }
)";

struct Vertex {
  float position[3];
};

bool test_fbx(std::vector<Vertex>& out_vertices) {
  out_vertices.push_back(Vertex{-0.5f, -0.5f, 0.0f});
  out_vertices.push_back(Vertex{ 0.5f, -0.5f, 0.0f});
  out_vertices.push_back(Vertex{ 0.0f,  0.5f, 0.0f});
  return true;
}

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
    800,
    600,
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

  std::vector<Vertex> vertices;
  if (argc < 2) {
    if (!test_fbx(vertices)) {
      return 1;
    }
  } else {
    if (!load_fbx(argv[1], vertices)) {
      return 1;
    }
  }

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

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glViewport(0, 0, 800, 600);

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
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    SDL_GL_SwapWindow(window);
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shaderProgram);

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
