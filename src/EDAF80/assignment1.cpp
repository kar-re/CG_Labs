#include "CelestialBody.hpp"
#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/ShaderProgramManager.hpp"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "parametric_shapes.hpp"

#include <imgui.h>
#include <math.h>
#include <stack>

#include <clocale>
#include <cstdlib>

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t) {
  return x * (1.f - t) + y * t;
}
int main() {
  std::setlocale(LC_ALL, "");

  using namespace std::literals::chrono_literals;

  //
  // Set up the framework
  //
  Bonobo framework;

  //
  // Set up the camera
  //
  InputHandler input_handler;
  FPSCameraf camera(0.5f * glm::half_pi<float>(),
                    static_cast<float>(config::resolution_x) /
                        static_cast<float>(config::resolution_y),
                    0.01f, 1000.0f);
  camera.mWorld.SetTranslate(glm::vec3(0.0f, 4.0f, 5.0f));
  camera.mWorld.LookAt(glm::vec3(0.0f));
  camera.mMouseSensitivity = glm::vec2(0.003f);
  camera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h

  //
  // Create the window
  //
  WindowManager &window_manager = framework.GetWindowManager();
  WindowManager::WindowDatum window_datum{input_handler,
                                          camera,
                                          config::resolution_x,
                                          config::resolution_y,
                                          0,
                                          0,
                                          0,
                                          0};
  GLFWwindow *window = window_manager.CreateGLFWWindow(
      "EDAF80: Assignment 1", window_datum, config::msaa_rate);
  if (window == nullptr) {
    LogError("Failed to get a window: exiting.");

    return EXIT_FAILURE;
  }

  bonobo::init();

  //
  // Load the sphere geometry
  //
  std::vector<bonobo::mesh_data> const objects =
      bonobo::loadObjects(config::resources_path("scenes/sphere.obj"));
  if (objects.empty()) {
    LogError("Failed to load the sphere geometry: exiting.");

    bonobo::deinit();

    return EXIT_FAILURE;
  }
  bonobo::mesh_data const &sphere = objects.front();
  auto const saturn_ring_shape =
      parametric_shapes::createCircleRing(0.675f, 0.45f, 80u, 8u);

  //
  // Create the shader program
  //
  ShaderProgramManager program_manager;
  GLuint celestial_body_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Celestial Body",
      {{ShaderType::vertex, "EDAF80/default.vert"},
       {ShaderType::fragment, "EDAF80/default.frag"}},
      celestial_body_shader);
  if (celestial_body_shader == 0u) {
    LogError(
        "Failed to generate the “Celestial Body” shader program: exiting.");

    bonobo::deinit();

    return EXIT_FAILURE;
  }
  GLuint celestial_ring_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Celestial Ring",
      {{ShaderType::vertex, "EDAF80/celestial_ring.vert"},
       {ShaderType::fragment, "EDAF80/celestial_ring.frag"}},
      celestial_ring_shader);
  if (celestial_ring_shader == 0u) {
    LogError(
        "Failed to generate the “Celestial Ring” shader program: exiting.");

    bonobo::deinit();

    return EXIT_FAILURE;
  }

  //
  // Define all the celestial bodies constants.
  //
  glm::vec3 const sun_scale{1.0f};
  SpinConfiguration const sun_spin{glm::radians(0.0f),
                                   glm::two_pi<float>() / 6.0f};

  glm::vec3 const mercury_scale{0.02f};
  SpinConfiguration const mercury_spin{glm::radians(-0.0f),
                                       glm::two_pi<float>() / 180.0f};
  OrbitConfiguration const mercury_orbit{2.0f, glm::radians(-3.4f),
                                         glm::two_pi<float>() / 4.0f};

  glm::vec3 const venus_scale{0.05f};
  SpinConfiguration const venus_spin{glm::radians(-2.6f),
                                     -glm::two_pi<float>() / 600.0f};
  OrbitConfiguration const venus_orbit{3.0f, glm::radians(-3.9f),
                                       glm::two_pi<float>() / 12.0f};

  glm::vec3 const earth_scale{0.05f};
  SpinConfiguration const earth_spin{glm::radians(-23.0f),
                                     glm::two_pi<float>() / 3.0f};
  OrbitConfiguration const earth_orbit{4.0f, glm::radians(-7.2f),
                                       glm::two_pi<float>() / 20.0f};

  glm::vec3 const moon_scale{0.01f};
  SpinConfiguration const moon_spin{glm::radians(-6.7f),
                                    glm::two_pi<float>() / 90.0f};
  OrbitConfiguration const moon_orbit{0.2f, glm::radians(29.0f),
                                      glm::two_pi<float>() / 1.3f};

  glm::vec3 const mars_scale{0.03f};
  SpinConfiguration const mars_spin{glm::radians(-25.0f),
                                    glm::two_pi<float>() / 3.0f};
  OrbitConfiguration const mars_orbit{5.0f, glm::radians(-5.7f),
                                      glm::two_pi<float>() / 36.0f};

  glm::vec3 const jupiter_scale{0.5f};
  SpinConfiguration const jupiter_spin{glm::radians(-3.1f),
                                       glm::two_pi<float>() / 1.0f};
  OrbitConfiguration const jupiter_orbit{13.0f, glm::radians(-6.1f),
                                         glm::two_pi<float>() / 220.0f};

  glm::vec3 const saturn_scale{0.4f};
  SpinConfiguration const saturn_spin{glm::radians(-27.0f),
                                      glm::two_pi<float>() / 1.2f};
  OrbitConfiguration const saturn_orbit{16.0f, glm::radians(-5.5f),
                                        glm::two_pi<float>() / 400.0f};
  glm::vec2 const saturn_ring_scale{1.0f, 1.25f};

  glm::vec3 const uranus_scale{0.2f};
  SpinConfiguration const uranus_spin{glm::radians(-82.0f),
                                      -glm::two_pi<float>() / 2.0f};
  OrbitConfiguration const uranus_orbit{18.0f, glm::radians(-6.5f),
                                        glm::two_pi<float>() / 1680.0f};

  glm::vec3 const neptune_scale{0.2f};
  SpinConfiguration const neptune_spin{glm::radians(-28.0f),
                                       glm::two_pi<float>() / 2.0f};
  OrbitConfiguration const neptune_orbit{19.0f, glm::radians(-6.4f),
                                         glm::two_pi<float>() / 3200.0f};

  //
  // Load all textures.
  //
  GLuint const sun_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_sun.jpg"));
  GLuint const mercury_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_mercury.jpg"));
  GLuint const venus_texture = bonobo::loadTexture2D(
      config::resources_path("planets/2k_venus_atmosphere.jpg"));
  GLuint const earth_texture = bonobo::loadTexture2D(
      config::resources_path("planets/2k_earth_daymap.jpg"));
  GLuint const moon_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_moon.jpg"));
  GLuint const mars_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_mars.jpg"));
  GLuint const jupiter_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_jupiter.jpg"));
  GLuint const saturn_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_saturn.jpg"));
  GLuint const saturn_ring_texture = bonobo::loadTexture2D(
      config::resources_path("planets/2k_saturn_ring_alpha.png"));
  GLuint const uranus_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_uranus.jpg"));
  GLuint const neptune_texture =
      bonobo::loadTexture2D(config::resources_path("planets/2k_neptune.jpg"));

  //
  // Set up the celestial bodies.
  //

  // moon.set_scale(glm::vec3(0.3f));
  // moon.set_orbit({1.5f, glm::radians(-66.0f), glm::two_pi<float>() / 1.3f});

  CelestialBody neptune(sphere, &celestial_body_shader, neptune_texture);
  neptune.set_spin(neptune_spin);
  neptune.set_orbit(neptune_orbit);
  neptune.set_scale(neptune_scale);

  CelestialBody uranus(sphere, &celestial_body_shader, uranus_texture);
  uranus.set_spin(uranus_spin);
  uranus.set_orbit(uranus_orbit);
  uranus.set_scale(uranus_scale);

  CelestialBody saturn(sphere, &celestial_body_shader, saturn_texture);
  saturn.set_spin(saturn_spin);
  saturn.set_orbit(saturn_orbit);
  saturn.set_scale(saturn_scale);
  saturn.set_ring(saturn_ring_shape, &celestial_ring_shader,
                  saturn_ring_texture, saturn_ring_scale);

  CelestialBody jupiter(sphere, &celestial_body_shader, jupiter_texture);
  jupiter.set_spin(jupiter_spin);
  jupiter.set_orbit(jupiter_orbit);
  jupiter.set_scale(jupiter_scale);

  CelestialBody mars(sphere, &celestial_body_shader, mars_texture);
  mars.set_spin(mars_spin);
  mars.set_orbit(mars_orbit);
  mars.set_scale(mars_scale);

  CelestialBody moon(sphere, &celestial_body_shader, moon_texture);
  moon.set_spin(moon_spin);
  moon.set_orbit(moon_orbit);
  moon.set_scale(moon_scale);

  CelestialBody earth(sphere, &celestial_body_shader, earth_texture);
  earth.set_spin(earth_spin);
  earth.set_orbit(earth_orbit);
  earth.set_scale(earth_scale);
  earth.add_child(&moon);

  CelestialBody venus(sphere, &celestial_body_shader, venus_texture);
  venus.set_spin(venus_spin);
  venus.set_orbit(venus_orbit);
  venus.set_scale(venus_scale);

  CelestialBody mercury(sphere, &celestial_body_shader, mercury_texture);
  mercury.set_spin(mercury_spin);
  mercury.set_orbit(mercury_orbit);
  mercury.set_scale(mercury_scale);

  CelestialBody sun(sphere, &celestial_body_shader, sun_texture);
  sun.set_spin(sun_spin);
  sun.set_scale(sun_scale);

  sun.add_child(&uranus);
  sun.add_child(&jupiter);
  sun.add_child(&neptune);
  sun.add_child(&saturn);
  sun.add_child(&mars);
  sun.add_child(&venus);
  sun.add_child(&mercury);
  sun.add_child(&earth);

  // earth.set_orbit({-2.5f, glm::radians(45.0f), glm::two_pi<float>()
  // / 10.0f});
  //
  // Define the colour and depth used for clearing.
  //
  // Alternatively, multiply everything with inverse of a given planet
  glClearDepthf(1.0f);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  auto last_time = std::chrono::high_resolution_clock::now();

  bool pause_animation = false;
  bool show_logs = true;
  bool show_gui = true;
  bool show_basis = false;
  float time_scale = 1.0f;

  int focus = 0, counter = 0;
  std::vector<glm::vec3> positions;
  bool is_focused = false;

  glm::mat4 focus_world_matrix = glm::mat4(1.0f);
  // earth.set_scale(glm::vec3(1.0, 0.2, 0.2));
  while (!glfwWindowShouldClose(window)) {
    //
    // Compute timings information
    //
    auto const now_time = std::chrono::high_resolution_clock::now();
    auto const delta_time_us =
        std::chrono::duration_cast<std::chrono::microseconds>(now_time -
                                                              last_time);
    auto const animation_delta_time_us =
        !pause_animation
            ? std::chrono::duration_cast<std::chrono::microseconds>(
                  delta_time_us * time_scale)
            : 0us;
    last_time = now_time;

    //
    // Process inputs
    //
    glfwPollEvents();

    ImGuiIO const &io = ImGui::GetIO();
    input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
    input_handler.Advance();

    camera.Update(delta_time_us, input_handler);

    if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
      show_logs = !show_logs;
    if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
      show_gui = !show_gui;
    if (input_handler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
      window_manager.ToggleFullscreenStatusForWindow(window);

    // Lerping on previous positions. Choose a reference, sample positions in an
    // vector. Lerp between, which gives smooth transition and smooth camera
    // movement.

    // Alternatively - global camera pose, i.e. independent view matrix.
    // Can be swapped as well, even if we have a stack.
    // Follow planets, randomize followed planet, switch isFocused
    if (input_handler.GetKeycodeState(GLFW_KEY_M) & JUST_RELEASED) {
      if (is_focused)
        focus++;
      if (focus > 9)
        focus = 0;
      LogInfo("Focus: (%i), isFocused (%i)", focus, is_focused);
      focus_world_matrix = glm::mat4(1.0);
      is_focused = !is_focused;
    }

    // Retrieve the actual framebuffer size: for HiDPI monitors,
    // you might end up with a framebuffer larger than what you
    // actually asked for. For example, if you ask for a 1920x1080
    // framebuffer, you might get a 3840x2160 one instead.
    // Also it might change as the user drags the window between
    // monitors with different DPIs, or if the fullscreen status is
    // being toggled.
    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    //
    // Start a new frame for Dear ImGui
    //
    window_manager.NewImGuiFrame();

    //
    // Clear the screen
    //
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //
    // Traverse the scene graph and render all nodes
    //

    // TODO: Replace this explicit rendering of the Earth and Moon
    // with a traversal of the scene graph and rendering of all its
    // nodes.
    // auto const parent = earth.render(animation_delta_time_us,
    // camera.GetWorldToClipMatrix(), glm::translate(glm::mat4(1.0f),
    // glm::vec3(2.0f, 0.0f, 0.0f)), show_basis);

    // moon.render(animation_delta_time_us, camera.GetWorldToClipMatrix(),
    // parent, show_basis);
    struct CelestialBodyRef {
      CelestialBody *body;
      glm::mat4 parent_transform;
    };
    std::stack<CelestialBodyRef> celestial_bodies;

    CelestialBodyRef sun_ref;
    sun_ref.body = &sun;
    sun_ref.parent_transform =
        glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    celestial_bodies.push(sun_ref);
    counter = 0;
    while (!celestial_bodies.empty()) {
      CelestialBodyRef &body_ref = celestial_bodies.top();
      celestial_bodies.pop();
      glm::mat4 transform = body_ref.body->render(
          animation_delta_time_us,
          camera.GetWorldToClipMatrix() * focus_world_matrix,
          body_ref.parent_transform, show_basis);
      if (is_focused && (counter == focus)) {

        glm::vec3 position = glm::vec3(transform[3]);
        // Get position of focused planet, the fourth column in transformation
        // matrix, set camera translate according to that.
        // camera.mWorld.SetTranslate(position + glm::vec3(0.0f, 0.2f, 0.5f));

        // Alternative method: get translation matrix for negative position,
        // multiply camera world to clip matrix with that!
        focus_world_matrix = glm::translate(glm::mat4(1.0), -position);

        // Legacy
        // focus_world_matrix = glm::inverse(transform);
        // glm::vec3 look_at_position = transform[3];
        // glm::vec3 current_camera_position =
        // glm::vec3(camera.GetWorldToClipMatrix()[3]); glm::vec3
        // lerped_position = lerp(current_camera_position, look_at_position,
        // 0.5f); camera.mWorld.setLoo(position);
        // camera.mWorld.LookAt(position);

        // LogInfo("Cam: (%f) (%f) (%f), lerp: (%f) (%f) (%f) ",
        // current_camera_position.x, current_camera_position.y,
        // current_camera_position.z, lerped_position.x, lerped_position.y,
        // lerped_position.z); LogInfo("Pos: (%f) (%f) (%f)", position.x,
        // position.y, position.z); camera.mWorld.SetTranslate(lerped_position);
        // camera.mWorld.LookAt(look_at_position);
      }

      counter++;
      std::vector<CelestialBody *> const &children =
          body_ref.body->get_children();
      for (const auto &child : children) {
        CelestialBodyRef child_ref;
        child_ref.body = child;
        child_ref.parent_transform = transform;
        celestial_bodies.push(child_ref);
      }
    }
    //
    // Add controls to the scene.
    //
    bool const opened =
        ImGui::Begin("Scene controls", nullptr, ImGuiWindowFlags_None);
    if (opened) {
      ImGui::Checkbox("Pause the animation", &pause_animation);
      ImGui::SliderFloat("Time scale", &time_scale, 1e-1f, 10.0f);
      ImGui::Separator();
      ImGui::Checkbox("Show basis", &show_basis);
      if (is_focused)
        ImGui::BulletText("Focused on (%i)", focus);
    }
    ImGui::End();

    //
    // Display Dear ImGui windows
    //
    if (show_logs)
      Log::View::Render();
    window_manager.RenderImGuiFrame(show_gui);

    //
    // Queue the computed frame for display on screen
    //
    glfwSwapBuffers(window);
  }

  glDeleteTextures(1, &neptune_texture);
  glDeleteTextures(1, &uranus_texture);
  glDeleteTextures(1, &saturn_ring_texture);
  glDeleteTextures(1, &saturn_texture);
  glDeleteTextures(1, &jupiter_texture);
  glDeleteTextures(1, &mars_texture);
  glDeleteTextures(1, &moon_texture);
  glDeleteTextures(1, &earth_texture);
  glDeleteTextures(1, &venus_texture);
  glDeleteTextures(1, &mars_texture);
  glDeleteTextures(1, &sun_texture);

  bonobo::deinit();

  return EXIT_SUCCESS;
}
