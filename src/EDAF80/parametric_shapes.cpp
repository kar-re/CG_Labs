#include "parametric_shapes.hpp"
#include "core/Log.h"
#include "core/helpers.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const height,
                              unsigned int const horizontal_split_count,
                              unsigned int const vertical_split_count) {
  auto const vertices = std::array<glm::vec3, 4>{
      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(width, 0.0f, 0.0f),
      glm::vec3(width, 0.0f, height), glm::vec3(0.0f, 0.0f, height)};

  auto const index_sets =
      std::array<glm::uvec3, 2>{glm::uvec3(0u, 1u, 2u), glm::uvec3(0u, 2u, 3u)};

  auto const bo_size =
      static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
  auto const ibo_size =
      static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3));

  bonobo::mesh_data data;

  if (horizontal_split_count > 0u || vertical_split_count > 0u) {

    auto tess_vertices =
        std::vector<glm::vec3>(horizontal_split_count * vertical_split_count);
    auto tess_index_sets = std::vector<glm::uvec3>(
        2u * (horizontal_split_count - 1) * (vertical_split_count));
    auto texcoords =
        std::vector<glm::vec3>(horizontal_split_count * vertical_split_count);
    auto normals =
        std::vector<glm::vec3>(horizontal_split_count * vertical_split_count);
    auto binormals =
        std::vector<glm::vec3>(horizontal_split_count * vertical_split_count);
    auto tangents =
        std::vector<glm::vec3>(horizontal_split_count * vertical_split_count);
    size_t index = 0u;

    for (unsigned int x = 0u; x < horizontal_split_count; x++) {
      for (unsigned int z = 0u; z < vertical_split_count; z++) {
        tess_vertices[index] =
            glm::vec3((width / horizontal_split_count) * x, 0,
                      (height / vertical_split_count) * z);

        texcoords[index] = glm::vec3(
            static_cast<float>(x) /
                (static_cast<float>(horizontal_split_count)),
            static_cast<float>(z) / (static_cast<float>(vertical_split_count)),
            0.0f);

        normals[index] = glm::vec3(0.0f, 1.0f, 0.0f);
        binormals[index] = glm::vec3(1.0f, 0.0f, 0.0f);
        tangents[index] = glm::vec3(0.0f, 0.0f, 1.0f);

        ++index;
      }
    }

    index = 0u;
    for (unsigned int x = 0u; x < horizontal_split_count - 1; x++) {
      for (unsigned int z = 0u; z < vertical_split_count - 1; z++) {
        tess_index_sets[index] = glm::uvec3(
            x * vertical_split_count + z, x * vertical_split_count + z + 1u,
            x * vertical_split_count + z + vertical_split_count);
        index++;

        tess_index_sets[index] = glm::uvec3(
            x * vertical_split_count + z + vertical_split_count,
            x * vertical_split_count + z + 1u,
            x * vertical_split_count + z + vertical_split_count + 1u);
        index++;
      }
    }

    glGenVertexArrays(1, &data.vao);
    assert(data.vao != 0u);
    glBindVertexArray(data.vao);

    auto const vertices_offset = 0u;
    auto const vertices_size =
        static_cast<GLsizeiptr>(tess_vertices.size() * sizeof(glm::vec3));
    auto const normals_offset = vertices_size;
    auto const normals_size =
        static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
    auto const texcoords_offset = normals_offset + normals_size;
    auto const texcoords_size =
        static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
    auto const tangents_offset = texcoords_offset + texcoords_size;
    auto const tangents_size =
        static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
    auto const binormals_offset = tangents_offset + tangents_size;
    auto const binormals_size =
        static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
    auto const bo_size =
        static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size +
                                tangents_size + binormals_size);
    glGenBuffers(1, &data.bo);
    assert(data.bo != 0u);
    glBindBuffer(GL_ARRAY_BUFFER, data.bo);
    glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size,
                    static_cast<GLvoid const *>(tess_vertices.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::vertices));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3,
        GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

    glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size,
                    static_cast<GLvoid const *>(texcoords.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3,
        GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<GLvoid const *>(texcoords_offset));

    glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size,
                    static_cast<GLvoid const *>(normals.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::normals));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::normals), 3,
        GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<GLvoid const *>(normals_offset));

    glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size,
                    static_cast<GLvoid const *>(texcoords.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3,
        GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<GLvoid const *>(texcoords_offset));

    glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size,
                    static_cast<GLvoid const *>(tangents.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::tangents));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3,
        GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<GLvoid const *>(tangents_offset));

    glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size,
                    static_cast<GLvoid const *>(binormals.data()));
    glEnableVertexAttribArray(
        static_cast<unsigned int>(bonobo::shader_bindings::binormals));
    glVertexAttribPointer(
        static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3,
        GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<GLvoid const *>(binormals_offset));

    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    data.indices_nb = tess_index_sets.size() * 3u;
    glGenBuffers(1, &data.ibo);
    assert(data.ibo != 0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(tess_index_sets.size() * sizeof(glm::uvec3)),
        reinterpret_cast<GLvoid const *>(tess_index_sets.data()),
        GL_STATIC_DRAW);

    glBindVertexArray(0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

    return data;
    // LogError("parametric_shapes::createQuad() does not support
    // tesselation.");
  }

  //
  // NOTE:
  //
  // Only the values preceeded by a `\todo` tag should be changed, the
  // other ones are correct!
  //

  // Create a Vertex Array Object: it will remember where we stored the
  // data on the GPU, and  which part corresponds to the vertices, which
  // one for the normals, etc.
  //
  // The following function will create new Vertex Arrays, and pass their
  // name in the given array (second argument). Since we only need one,
  // pass a pointer to `data.vao`.
  glGenVertexArrays(1, &data.vao);

  // To be able to store information, the Vertex Array has to be bound
  // first.
  glBindVertexArray(data.vao);

  // To store the data, we need to allocate buffers on the GPU. Let's
  // allocate a first one for the vertices.
  //
  // The following function's syntax is similar to `glGenVertexArray()`:
  // it will create multiple OpenGL objects, in this case buffers, and
  // return their names in an array. Have the buffer's name stored into
  // `data.bo`.
  glGenBuffers(1, &data.bo);

  // Similar to the Vertex Array, we need to bind it first before storing
  // anything in it. The data stored in it can be interpreted in
  // different ways. Here, we will say that it is just a simple 1D-array
  // and therefore bind the buffer to the corresponding target.
  glBindBuffer(GL_ARRAY_BUFFER, data.bo);

  glBufferData(
      GL_ARRAY_BUFFER, bo_size,
      /* where is the data stored on the CPU? */ vertices.data(),
      /* inform OpenGL that the data is modified once, but used often */
      GL_STATIC_DRAW);

  // Vertices have been just stored into a buffer, but we still need to
  // tell Vertex Array where to find them, and how to interpret the data
  // within that buffer.
  //
  // You will see shaders in more detail in lab 3, but for now they are
  // just pieces of code running on the GPU and responsible for moving
  // all the vertices to clip space, and assigning a colour to each pixel
  // covered by geometry.
  // Those shaders have inputs, some of them are the data we just stored
  // in a buffer object. We need to tell the Vertex Array which inputs
  // are enabled, and this is done by the following line of code, which
  // enables the input for vertices:
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices));

  // Once an input is enabled, we need to explain where the data comes
  // from, and how it interpret it. When calling the following function,
  // the Vertex Array will automatically use the current buffer bound to
  // GL_ARRAY_BUFFER as its source for the data. How to interpret it is
  // specified below:
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices),
      /*! \todo how many components do our vertices have? */ 3,
      /* what is the type of each component? */ GL_FLOAT,
      /* should it automatically normalise the values stored */ GL_FALSE,
      /* once all components of a vertex have been read, how far away (in bytes)
         is the next vertex? */
      0,
      /* how far away (in bytes) from the start of the buffer is the first
         vertex? */
      reinterpret_cast<GLvoid const *>(0x0));

  // Now, let's allocate a second one for the indices.
  //
  // Have the buffer's name stored into `data.ibo`.
  glGenBuffers(1, /*! \todo fill me */ &data.ibo);

  // We still want a 1D-array, but this time it should be a 1D-array of
  // elements, aka. indices!
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
               /*! \todo bind the previously generated Buffer */ data.ibo);

  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      /*! \todo how many bytes should the buffer contain? */ ibo_size,
      /* where is the data stored on the CPU? */ index_sets.data(),
      /* inform OpenGL that the data is modified once, but used often */
      GL_STATIC_DRAW);

  data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);

  // All the data has been recorded, we can unbind them.
  glBindVertexArray(0u);
  glBindBuffer(GL_ARRAY_BUFFER, 0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

  return data;
}

/// Creates a sphere
///
/// \param radius Sphere radius
/// \param longitude_split_count Amounts of split in the vertical direction
/// \param latitude_split_count Amounts of split in the horizontal direction
/// \return mesh data
bonobo::mesh_data
parametric_shapes::createSphere(float const radius,
                                unsigned int const longitude_split_count,
                                unsigned int const latitude_split_count) {
  // Calculate number of edges, verts for lat and long
  auto const longitude_edges_count = longitude_split_count + 1u;
  auto const latitude_edges_count = latitude_split_count + 1u;
  auto const longitude_vertices_count = longitude_edges_count + 1u;
  auto const latitude_vertices_count = latitude_edges_count + 1u;
  auto const vertices_nb = longitude_vertices_count * latitude_vertices_count;

  // Create vectors for vert, normal, tex, tangent, binormal
  auto vertices = std::vector<glm::vec3>(vertices_nb);
  auto normals = std::vector<glm::vec3>(vertices_nb);
  auto texcoords = std::vector<glm::vec3>(vertices_nb);
  auto tangents = std::vector<glm::vec3>(vertices_nb);
  auto binormals = std::vector<glm::vec3>(vertices_nb);

  // step size for theta and phi, long lat resp.
  float const d_theta =
      glm::two_pi<float>() / (static_cast<float>(longitude_edges_count));
  float const d_phi =
      glm::pi<float>() / (static_cast<float>(latitude_edges_count));

  // generate vertices iteratively
  size_t index = 0u;
  float theta = 0.0f; // 0<theta<2pi - longitude
  for (unsigned int i = 0u; i < longitude_vertices_count; ++i) {
    float const cos_theta = std::cos(theta);
    float const sin_theta = std::sin(theta);
    // Theta is circular slices, from above

    float phi = 0.0f; // 0<phi<pi - latitude, reset for each slize
    for (unsigned int j = 0u; j < latitude_vertices_count; ++j) {
      float const cos_phi = std::cos(phi);
      float const sin_phi = std::sin(phi);

      // vertex, follows formula
      vertices[index] =
          glm::vec3(radius * sin_theta * sin_phi, -radius * cos_phi,
                    radius * cos_theta * sin_phi);

      // texture coordinates
      texcoords[index] = glm::vec3(
          static_cast<float>(i) /
              (static_cast<float>(longitude_vertices_count)),
          static_cast<float>(j) / (static_cast<float>(latitude_vertices_count)),
          0.0f);

      // tangent
      auto const t = glm::vec3(cos_theta, 0.0f, -sin_theta);
      //   auto const t = glm::vec3(radius * cos_theta * sin_phi, 0.0f,
      //                            -radius * sin_theta * sin_phi);
      //   auto const t = glm::vec3(radius * cos_theta_sin_phi, 0.0f,
      //                            -radius * sin_theta_sin_phi);
      tangents[index] = t;

      // binormal
      auto const b =
          glm::vec3(sin_theta * cos_phi, sin_phi, cos_theta * cos_phi);
      binormals[index] = b;

      // normal
      auto const n = glm::cross(t, b);
      normals[index] = n;

      phi += d_phi;
      ++index;
    }

    theta += d_theta;
  }
  //   printf("\n Generated verticies and stuff..., length %u \n",
  //   vertices.size());

  // create index array
  auto index_sets = std::vector<glm::uvec3>(2u * longitude_edges_count *
                                            latitude_edges_count);

  // generate indices iteratively
  index = 0u;
  for (unsigned int i = 0u; i < longitude_edges_count; ++i) {
    for (unsigned int j = 0u; j < latitude_edges_count; ++j) {
      // In order to change from clock wise to counter clock wise
      // Switch order of the Y and Z component in index array.
      index_sets[index] =
          glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),
                     latitude_vertices_count * (i + 1u) + (j + 1u),
                     latitude_vertices_count * (i + 0u) + (j + 1u));
      ++index;

      index_sets[index] =
          glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),
                     latitude_vertices_count * (i + 1u) + (j + 0u),
                     latitude_vertices_count * (i + 1u) + (j + 1u));
      ++index;
    }
  }

  bonobo::mesh_data data;
  glGenVertexArrays(1, &data.vao);
  assert(data.vao != 0u);
  glBindVertexArray(data.vao);

  auto const vertices_offset = 0u;
  auto const vertices_size =
      static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
  auto const normals_offset = vertices_size;
  auto const normals_size =
      static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
  auto const texcoords_offset = normals_offset + normals_size;
  auto const texcoords_size =
      static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
  auto const tangents_offset = texcoords_offset + texcoords_size;
  auto const tangents_size =
      static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
  auto const binormals_offset = tangents_offset + tangents_size;
  auto const binormals_size =
      static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
  auto const bo_size =
      static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size +
                              tangents_size + binormals_size);
  glGenBuffers(1, &data.bo);
  assert(data.bo != 0u);
  glBindBuffer(GL_ARRAY_BUFFER, data.bo);
  glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size,
                  static_cast<GLvoid const *>(vertices.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

  glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size,
                  static_cast<GLvoid const *>(normals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::normals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(normals_offset));

  glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size,
                  static_cast<GLvoid const *>(texcoords.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(texcoords_offset));

  glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size,
                  static_cast<GLvoid const *>(tangents.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(tangents_offset));

  glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size,
                  static_cast<GLvoid const *>(binormals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(binormals_offset));

  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
  glGenBuffers(1, &data.ibo);
  assert(data.ibo != 0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)),
               reinterpret_cast<GLvoid const *>(index_sets.data()),
               GL_STATIC_DRAW);

  glBindVertexArray(0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

  return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count) {

  auto const longitude_edges_count = major_split_count + 1u;
  auto const latitude_edges_count = minor_split_count + 1u;
  auto const longitude_vertices_count = longitude_edges_count + 1u;
  auto const latitude_vertices_count = latitude_edges_count + 1u;
  auto const vertices_nb = longitude_vertices_count * latitude_vertices_count;

  // Create vectors for vert, normal, tex, tangent, binormal
  auto vertices = std::vector<glm::vec3>(vertices_nb);
  auto normals = std::vector<glm::vec3>(vertices_nb);
  auto texcoords = std::vector<glm::vec3>(vertices_nb);
  auto tangents = std::vector<glm::vec3>(vertices_nb);
  auto binormals = std::vector<glm::vec3>(vertices_nb);

  // step size for theta and phi, long lat resp.
  float const d_theta =
      glm::two_pi<float>() / (static_cast<float>(longitude_edges_count));
  float const d_phi =
      glm::two_pi<float>() / (static_cast<float>(latitude_edges_count));

  // generate vertices iteratively
  size_t index = 0u;
  float theta = 0.0f; // 0<theta<2pi - longitude
  for (unsigned int i = 0u; i < longitude_vertices_count; ++i) {
    float const cos_theta = std::cos(theta);
    float const sin_theta = std::sin(theta);
    // Theta is circular slices, from above

    float phi = 0.0f; // 0<phi<2pi - each slize
    for (unsigned int j = 0u; j < latitude_vertices_count; ++j) {
      float const cos_phi = std::cos(phi);
      float const sin_phi = std::sin(phi);
      // vertex, follows formula
      vertices[index] =
          glm::vec3((major_radius + minor_radius * cos_theta) * cos_phi,
                    -minor_radius * sin_theta,
                    (major_radius + minor_radius * cos_theta) * sin_phi);

      // texture coordinates
      texcoords[index] = glm::vec3(
          static_cast<float>(j) /
              (static_cast<float>(longitude_vertices_count)),
          static_cast<float>(i) / (static_cast<float>(latitude_vertices_count)),
          0.0f);

      // tangent
      auto const t = glm::vec3(minor_radius * sin_theta * cos_phi,
                               minor_radius * cos_theta,
                               minor_radius * sin_theta * sin_phi);
      tangents[index] = t;

      // binormal
      auto const b =
          glm::vec3(-(major_radius + minor_radius * cos_theta) * sin_phi, 0.0f,
                    (major_radius + minor_radius * cos_theta) * cos_phi);
      binormals[index] = b;

      // normal
      auto const n = glm::cross(t, b);
      normals[index] = n;

      phi += d_phi;
      ++index;
    }

    theta += d_theta;
  }
  //   printf("\n Generated verticies and stuff..., length %u \n",
  //   vertices.size());

  // create index array
  auto index_sets = std::vector<glm::uvec3>(2u * longitude_edges_count *
                                            latitude_edges_count);

  // generate indices iteratively
  index = 0u;
  for (unsigned int i = 0u; i < longitude_edges_count; ++i) {
    for (unsigned int j = 0u; j < latitude_edges_count; ++j) {
      // In order to change from clock wise to counter clock wise
      // Switch order of the Y and Z component in index array.
      index_sets[index] =
          glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),
                     latitude_vertices_count * (i + 0u) + (j + 1u),
                     latitude_vertices_count * (i + 1u) + (j + 1u));
      ++index;

      index_sets[index] =
          glm::uvec3(latitude_vertices_count * (i + 0u) + (j + 0u),
                     latitude_vertices_count * (i + 1u) + (j + 1u),
                     latitude_vertices_count * (i + 1u) + (j + 0u));
      ++index;
    }
  }

  bonobo::mesh_data data;
  glGenVertexArrays(1, &data.vao);
  assert(data.vao != 0u);
  glBindVertexArray(data.vao);

  auto const vertices_offset = 0u;
  auto const vertices_size =
      static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
  auto const normals_offset = vertices_size;
  auto const normals_size =
      static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
  auto const texcoords_offset = normals_offset + normals_size;
  auto const texcoords_size =
      static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
  auto const tangents_offset = texcoords_offset + texcoords_size;
  auto const tangents_size =
      static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
  auto const binormals_offset = tangents_offset + tangents_size;
  auto const binormals_size =
      static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
  auto const bo_size =
      static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size +
                              tangents_size + binormals_size);
  glGenBuffers(1, &data.bo);
  assert(data.bo != 0u);
  glBindBuffer(GL_ARRAY_BUFFER, data.bo);
  glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size,
                  static_cast<GLvoid const *>(vertices.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

  glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size,
                  static_cast<GLvoid const *>(normals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::normals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(normals_offset));

  glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size,
                  static_cast<GLvoid const *>(texcoords.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(texcoords_offset));

  glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size,
                  static_cast<GLvoid const *>(tangents.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(tangents_offset));

  glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size,
                  static_cast<GLvoid const *>(binormals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(binormals_offset));

  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
  glGenBuffers(1, &data.ibo);
  assert(data.ibo != 0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)),
               reinterpret_cast<GLvoid const *>(index_sets.data()),
               GL_STATIC_DRAW);

  glBindVertexArray(0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

  return data;
}

bonobo::mesh_data parametric_shapes::createCircleRing(
    float const radius, float const spread_length,
    unsigned int const split_count, unsigned int const spread_split_count) {
  auto const slice_edges_count = split_count + 1u;
  auto const spread_slice_edges_count = spread_split_count + 1u;
  auto const slice_vertices_count = slice_edges_count + 1u;
  auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
  auto const vertices_nb = slice_vertices_count * spread_slice_vertices_count;

  auto vertices = std::vector<glm::vec3>(vertices_nb);
  auto normals = std::vector<glm::vec3>(vertices_nb);
  auto texcoords = std::vector<glm::vec3>(vertices_nb);
  auto tangents = std::vector<glm::vec3>(vertices_nb);
  auto binormals = std::vector<glm::vec3>(vertices_nb);

  float const spread_start = radius - 0.5f * spread_length;
  float const d_theta =
      glm::two_pi<float>() / (static_cast<float>(slice_edges_count));
  float const d_spread =
      spread_length / (static_cast<float>(spread_slice_edges_count));

  // generate vertices iteratively
  size_t index = 0u;
  float theta = 0.0f;
  for (unsigned int i = 0u; i < slice_vertices_count; ++i) {
    float const cos_theta = std::cos(theta);
    float const sin_theta = std::sin(theta);

    float distance_to_centre = spread_start;
    for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
      // vertex
      vertices[index] = glm::vec3(distance_to_centre * cos_theta,
                                  distance_to_centre * sin_theta, 0.0f);

      // texture coordinates
      texcoords[index] = glm::vec3(
          static_cast<float>(j) /
              (static_cast<float>(spread_slice_vertices_count)),
          static_cast<float>(i) / (static_cast<float>(slice_vertices_count)),
          0.0f);

      // tangent
      auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
      tangents[index] = t;

      // binormal
      auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
      binormals[index] = b;

      // normal
      auto const n = glm::cross(t, b);
      normals[index] = n;

      distance_to_centre += d_spread;
      ++index;
    }

    theta += d_theta;
  }

  // create index array
  auto index_sets = std::vector<glm::uvec3>(2u * slice_edges_count *
                                            spread_slice_edges_count);

  // generate indices iteratively
  index = 0u;
  for (unsigned int i = 0u; i < slice_edges_count; ++i) {
    for (unsigned int j = 0u; j < spread_slice_edges_count; ++j) {
      index_sets[index] =
          glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
                     spread_slice_vertices_count * (i + 0u) + (j + 1u),
                     spread_slice_vertices_count * (i + 1u) + (j + 1u));
      ++index;

      index_sets[index] =
          glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
                     spread_slice_vertices_count * (i + 1u) + (j + 1u),
                     spread_slice_vertices_count * (i + 1u) + (j + 0u));
      ++index;
    }
  }

  bonobo::mesh_data data;
  glGenVertexArrays(1, &data.vao);
  assert(data.vao != 0u);
  glBindVertexArray(data.vao);

  auto const vertices_offset = 0u;
  auto const vertices_size =
      static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
  auto const normals_offset = vertices_size;
  auto const normals_size =
      static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
  auto const texcoords_offset = normals_offset + normals_size;
  auto const texcoords_size =
      static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
  auto const tangents_offset = texcoords_offset + texcoords_size;
  auto const tangents_size =
      static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
  auto const binormals_offset = tangents_offset + tangents_size;
  auto const binormals_size =
      static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
  auto const bo_size =
      static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size +
                              tangents_size + binormals_size);
  glGenBuffers(1, &data.bo);
  assert(data.bo != 0u);
  glBindBuffer(GL_ARRAY_BUFFER, data.bo);
  glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size,
                  static_cast<GLvoid const *>(vertices.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

  glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size,
                  static_cast<GLvoid const *>(normals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::normals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(normals_offset));

  glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size,
                  static_cast<GLvoid const *>(texcoords.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(texcoords_offset));

  glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size,
                  static_cast<GLvoid const *>(tangents.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT,
      GL_FALSE, 0, reinterpret_cast<GLvoid const *>(tangents_offset));

  glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size,
                  static_cast<GLvoid const *>(binormals.data()));
  glEnableVertexAttribArray(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals));
  glVertexAttribPointer(
      static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3,
      GL_FLOAT, GL_FALSE, 0,
      reinterpret_cast<GLvoid const *>(binormals_offset));

  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
  glGenBuffers(1, &data.ibo);
  assert(data.ibo != 0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)),
               reinterpret_cast<GLvoid const *>(index_sets.data()),
               GL_STATIC_DRAW);

  glBindVertexArray(0u);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

  return data;
}
