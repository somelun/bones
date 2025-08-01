#include <iostream>

#include "ufbx.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

struct Vertex {
  float position[3];
};

bool load_fbx(const char* path, std::vector<Vertex>& out_vertices) {
  ufbx_load_opts opts = {0};
  ufbx_error error;
  ufbx_scene *scene = ufbx_load_file(path, &opts, &error);

  if (!scene) {
    fprintf(stderr, "Failed to load: %s\n", error.description.data);
    return 1;
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

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
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

  if (argc < 2) {
    std::cerr << "No input parameters" << std::endl;
    return 1;
  }

  std::vector<Vertex> vertices;
  if (!load_fbx(argv[1], vertices)) {
    return 1;
  }

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)0);

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

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());

    SDL_GL_SwapWindow(window);
  }

  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
