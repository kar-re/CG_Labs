#pragma once

#include "core/FPSCamera.h"
#include "core/InputHandler.h"
#include "core/WindowManager.hpp"
#include <glm/vec3.hpp>

class Window;

namespace edaf80 {
//! \brief Wrapper class for Assignment 5
class Assignment5 {
public:
  //! \brief Default constructor.
  //!
  //! It will initialise various modules of bonobo and retrieve a
  //! window to draw to.
  Assignment5(WindowManager &windowManager);

  //! \brief Default destructor.
  //!
  //! It will release the bonobo modules initialised by the
  //! constructor, as well as the window.
  ~Assignment5();

  //! \brief Contains the logic of the assignment, along with the
  //! render loop.
  void run();

  glm::vec3 random_position(float scale);

private:
  FPSCameraf mCamera;
  InputHandler inputHandler;
  WindowManager &mWindowManager;
  GLFWwindow *window;
};
} // namespace edaf80
