#include "interpolation.hpp"
#include <glm/fwd.hpp>

glm::vec3 interpolation::evalLERP(glm::vec3 const &p0, glm::vec3 const &p1,
                                  float const x) {
  return (1.0f - x) * p0 + (x * p1);
}

glm::vec3 interpolation::evalCatmullRom(glm::vec3 const &p0,
                                        glm::vec3 const &p1,
                                        glm::vec3 const &p2,
                                        glm::vec3 const &p3, float const t,
                                        float const x) {
  glm::vec4 const smooth = glm::vec4(1.0f, x, x * x, x * x * x);
  glm::mat4 const ding = {0.0f,
                          1.0f,
                          0.0f,
                          0.0f,
                          -t,
                          0,
                          t,
                          0,
                          2.0f * t,
                          t - 3.0f,
                          3.0f - 2.0f * t,
                          -t,
                          -t,
                          2.0f - t,
                          t - 2.0f,
                          t};
  auto const t01 = std::pow(glm::distance(p0, p1), x);
  auto const t12 = std::pow(glm::distance(p1, p2), x);
  auto const t23 = std::pow(glm::distance(p2, p3), x);

  glm::vec3 const m1 =
      (1.0f - t) *
      (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
  glm::vec3 const m2 =
      (1.0f - t) *
      (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));
  glm::vec3 const result =
      glm::vec3(2.0f * (p1 - p2) + m1 + m2 * t * t * t + -3.0f * (p1 - p2) -
                m1 - m1 - m2 * t * t + m1 * t + p1);

  glm::vec3 const result2 =
      (-t * x + 2.0f * t * x * x - t * x * x * x) * p0 +
      (1 + (t - 3.0f) * x * x + (2.0f - t) * x * x * x) * p1 +
      (t * x + (3.0f - 2.0f * t) * x * x + (t - 2.0f) * x * x * x) * p2 +
      (-(x * x * t) + x * x * x * t) * p3;

  return result2;
}
