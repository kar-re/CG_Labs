#include "assignment5.hpp"
#include "core/InputHandler.h"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/ShaderProgramManager.hpp"
#include "core/helpers.hpp"
#include "core/node.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <clocale>
#include <stdexcept>

edaf80::Assignment5::Assignment5(WindowManager &windowManager)
    : mCamera(0.5f * glm::half_pi<float>(),
              static_cast<float>(config::resolution_x) /
                  static_cast<float>(config::resolution_y),
              0.01f, 1000.0f),
      inputHandler(), mWindowManager(windowManager), window(nullptr) {
  WindowManager::WindowDatum window_datum{inputHandler,
                                          mCamera,
                                          config::resolution_x,
                                          config::resolution_y,
                                          0,
                                          0,
                                          0,
                                          0};

  window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum,
                                           config::msaa_rate);
  if (window == nullptr) {
    throw std::runtime_error("Failed to get a window: aborting!");
  }

  bonobo::init();
}

edaf80::Assignment5::~Assignment5() { bonobo::deinit(); }

void edaf80::Assignment5::run() {
  // Set up the camera
  mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
  mCamera.mMouseSensitivity = glm::vec2(0.003f);
  mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
  auto camera_position = mCamera.mWorld.GetTranslation();

  // Create the shader programs
  ShaderProgramManager program_manager;
  GLuint fallback_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Fallback",
      {{ShaderType::vertex, "common/fallback.vert"},
       {ShaderType::fragment, "common/fallback.frag"}},
      fallback_shader);
  if (fallback_shader == 0u) {
    LogError("Failed to load fallback shader");
    return;
  }

  GLuint diffuse_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Diffuse",
      {{ShaderType::vertex, "EDAF80/diffuse.vert"},
       {ShaderType::fragment, "EDAF80/diffuse.frag"}},
      diffuse_shader);
  if (diffuse_shader == 0u)
    LogError("Failed to load diffuse shader");

  GLuint normal_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Normal",
      {{ShaderType::vertex, "EDAF80/normal.vert"},
       {ShaderType::fragment, "EDAF80/normal.frag"}},
      normal_shader);
  if (normal_shader == 0u)
    LogError("Failed to load normal shader");

  GLuint texcoord_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Texture coords",
      {{ShaderType::vertex, "EDAF80/texcoord.vert"},
       {ShaderType::fragment, "EDAF80/texcoord.frag"}},
      texcoord_shader);
  if (texcoord_shader == 0u)
    LogError("Failed to load texcoord shader");

  GLuint phong_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Phong",
      {{ShaderType::vertex, "EDAF80/phong.vert"},
       {ShaderType::fragment, "EDAF80/phong.frag"}},
      phong_shader);
  if (phong_shader == 0u)
    LogError("Failed to load phong shader");

  GLuint skybox_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Skybox",
      {{ShaderType::vertex, "EDAF80/cubemap.vert"},
       {ShaderType::fragment, "EDAF80/cubemap.frag"}},
      skybox_shader);
  if (skybox_shader == 0u)
    LogError("Failed to load skybox shader");

  GLuint water_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Water",
      {{ShaderType::vertex, "EDAF80/water.vert"},
       {ShaderType::fragment, "EDAF80/water.frag"}},
      water_shader);
  if (water_shader == 0u)
    LogError("Failed to load water shader");
  float elapsed_time_s = 0.0f;

  auto light_position = glm::vec3(-16.0f, 4.0f, 16.0f);
  auto const set_uniforms = [&light_position, &camera_position,
                             &elapsed_time_s](GLuint program) {
    glUniform3fv(glGetUniformLocation(program, "light_position"), 1,
                 glm::value_ptr(light_position));
    glUniform3fv(glGetUniformLocation(program, "camera_position"), 1,
                 glm::value_ptr(camera_position));
    glUniform1f(glGetUniformLocation(program, "t"), elapsed_time_s);
  };

  auto my_cube_map_id = bonobo::loadTextureCubeMap(
      config::resources_path("cubemaps/NissiBeach2/posx.jpg"),
      config::resources_path("cubemaps/NissiBeach2/negx.jpg"),
      config::resources_path("cubemaps/NissiBeach2/posy.jpg"),
      config::resources_path("cubemaps/NissiBeach2/negy.jpg"),
      config::resources_path("cubemaps/NissiBeach2/posz.jpg"),
      config::resources_path("cubemaps/NissiBeach2/negz.jpg"));

  auto normal_map_id =
      bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

  auto const diffuse_texture = bonobo::loadTexture2D(
      config::resources_path("textures/leather_red_02_coll1_2k.jpg"));
  auto const normal_texture = bonobo::loadTexture2D(
      config::resources_path("textures/leather_red_02_nor_2k.jpg"));
  auto const roughness_texture = bonobo::loadTexture2D(
      config::resources_path("textures/leather_red_02_rough_2k.jpg"));
  //
  // Todo: Load your geometry
  //

  auto skybox_shape = parametric_shapes::createSphere(500.0f, 100u, 100u);
  if (skybox_shape.vao == 0u) {
    LogError("Failed to retrieve the mesh for the skybox");
    return;
  }
  Node skybox;
  skybox.set_geometry(skybox_shape);
  skybox.set_program(&skybox_shader, set_uniforms);
  skybox.add_texture("my_cube_map", my_cube_map_id, GL_TEXTURE_CUBE_MAP);

  auto bee_shape = parametric_shapes::createSphere(1.0f, 32u, 64u);
  if (bee_shape.vao == 0u) {
    LogError("Failed to retrieve the mesh for the bee");
    return;
  }
  Node bee;
  bee.set_geometry(bee_shape);
  bee.set_program(&phong_shader, set_uniforms);
  bee.add_texture("diffuse_texture", diffuse_texture, GL_TEXTURE_2D);
  bee.add_texture("normal_texture", normal_texture, GL_TEXTURE_2D);
  bee.add_texture("roughness_texture", roughness_texture, GL_TEXTURE_2D);
  bee.get_transform().SetTranslate(glm::vec3(0.0f, 0.0f, 0.0f));
  // skybox.add_texture("my_cube_map", my_cube_map_id, GL_TEXTURE_CUBE_MAP);
  // skybox.get_transform().SetTranslate(glm::vec3(-50.0f, -50.0f, 50.0f))

  glClearDepthf(1.0f);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  auto lastTime = std::chrono::high_resolution_clock::now();

  bool show_logs = true;
  bool show_gui = true;
  bool shader_reload_failed = false;
  bool show_basis = false;
  bool game_started = false;
  float basis_thickness_scale = 1.0f;
  float basis_length_scale = 1.0f;
  float current_acceleration = 0.0f;
  float current_angle = 0.0f;

  while (!glfwWindowShouldClose(window)) {
    auto const nowTime = std::chrono::high_resolution_clock::now();
    auto const deltaTimeUs =
        std::chrono::duration_cast<std::chrono::microseconds>(nowTime -
                                                              lastTime);
    lastTime = nowTime;
    auto const deltaTime_s = std::chrono::duration<float>(deltaTimeUs);

    // logInf

    auto &io = ImGui::GetIO();
    inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

    glfwPollEvents();
    inputHandler.Advance();
    mCamera.Update(deltaTimeUs, inputHandler, true, false);
    camera_position = mCamera.mWorld.GetTranslation();

    if (game_started) {
      bee.get_transform().Translate(glm::vec3(glm::sin(current_angle) * 0.05f,
                                              current_acceleration,
                                              glm::cos(current_angle) * 0.05f));

      if (inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & JUST_RELEASED) {
        current_angle += 0.2f;
      }
      if (inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & JUST_RELEASED) {
        current_angle -= 0.2f;
      }
      if (inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & JUST_RELEASED)
        current_acceleration += 0.1f;

      current_acceleration -= 0.0982f * deltaTime_s.count();
    } else {
      if (inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & JUST_RELEASED) {
        game_started = true;
      }
    }

    if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
      shader_reload_failed = !program_manager.ReloadAllPrograms();
      if (shader_reload_failed)
        tinyfd_notifyPopup("Shader Program Reload Error",
                           "An error occurred while reloading shader programs; "
                           "see the logs for details.\n"
                           "Rendering is suspended until the issue is solved. "
                           "Once fixed, just reload the shaders again.",
                           "error");
    }

    if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
      show_logs = !show_logs;
    if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
      show_gui = !show_gui;
    if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
      mWindowManager.ToggleFullscreenStatusForWindow(window);

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
    // Todo: If you need to handle inputs, you can do it here
    //

    mWindowManager.NewImGuiFrame();

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (!shader_reload_failed) {
      //
      // Todo: Render all your geometry here.
      //
      skybox.render(mCamera.GetWorldToClipMatrix());
      bee.render(mCamera.GetWorldToClipMatrix());
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //
    // Todo: If you want a custom ImGUI window, you can set it up
    //       here
    //
    bool const opened =
        ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
    if (!game_started) {
      ImGui::Text("Press space to start game");
    }
    if (opened) {
      ImGui::Checkbox("Show basis", &show_basis);
      ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f,
                         100.0f);
      ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f,
                         100.0f);
      ImGui::Text("Delta time: %f", deltaTime_s.count());
    }
    ImGui::End();

    if (show_basis)
      bonobo::renderBasis(basis_thickness_scale, basis_length_scale,
                          mCamera.GetWorldToClipMatrix());
    if (show_logs)
      Log::View::Render();
    mWindowManager.RenderImGuiFrame(show_gui);

    glfwSwapBuffers(window);
  }
}

int main() {
  std::setlocale(LC_ALL, "");

  Bonobo framework;

  try {
    edaf80::Assignment5 assignment5(framework.GetWindowManager());
    assignment5.run();
  } catch (std::runtime_error const &e) {
    LogError(e.what());
  }
}
