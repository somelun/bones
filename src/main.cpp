#include <iostream>
#include "ufbx.h"

int main() {
  ufbx_load_opts opts = {0};
  ufbx_error error;
  ufbx_scene *scene = ufbx_load_file("model.fbx", &opts, &error);

  if (!scene) {
    fprintf(stderr, "Failed to load: %s\n", error.description.data);
    return 1;
  }

  ufbx_free_scene(scene);

  return 0;
}
