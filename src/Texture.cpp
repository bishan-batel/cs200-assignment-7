/**
 * Name: Kishan S Patel
 * Email: kishan.patel@digipen.edu
 * Assignment Number: 5
 * Course: CS200
 * Term: Fall 2024
 *
 * File: Texture.cpp
 *
 */
#include "Texture.h"
#include "Affine.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <utility>

using usize = std::size_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using i32 = std::int32_t;
using f32 = float;

namespace cs200 {

struct Color final {
  u8 r{0};
  u8 g{0};
  u8 b{0};

  constexpr Color(u8 r, u8 g, u8 b) : r{r}, g{g}, b{b} {}
};

static constexpr u32 COLOR_BYTES = sizeof(Color);
static constexpr u32 BYTE_BOUNDRY = 4;

static_assert(sizeof(char) == sizeof(u8), "Byte sizes do not match");

namespace header {
static constexpr usize SIZE = 54;

template <usize Offset, typename T> struct field {
  static constexpr usize offset = Offset;
  using type = T;

  constexpr field() = default;

  static_assert(
      Offset < SIZE,
      "Field has an offset that is outside the size of a bitmap header");
};

struct file_type final : field<0, u16> {};
struct file_size final : field<2, u32> {};
struct reserved1 final : field<6, u16> {};
struct reserved2 final : field<8, u16> {};
struct bitmap_offset final : field<10, u32> {};
struct header_size final : field<14, u32> {};
struct width final : field<18, i32> {};
struct height final : field<22, i32> {};
struct bit_planes final : field<26, u16> {};
struct bits_per_pixel final : field<28, u16> {};
struct compression final : field<30, u32> {};
struct size_of_bitmap final : field<34, u32> {};
struct horizantal_resolution final : field<38, u32> {};
struct vertical_resolution final : field<42, u32> {};
struct colors_used final : field<46, u32> {};
struct color_important final : field<50, u32> {};

template <typename Field>
constexpr typename Field::type get(const u8 *const buffer) {
  return *reinterpret_cast<const typename Field::type *>(
      &buffer[Field::offset]);
}
}; // namespace header

Bitmap::Bitmap(const u32 width, const u32 height)
    : bmp_data{}, bmp_width{width}, bmp_height{height},
      bmp_stride{computeStride(width)} {

  // reserve enough space
  bmp_data.reserve(bmp_stride * height * COLOR_BYTES);

  const glm::mat4 bitmap_to_tex = bitmapToTextureTransform(*this);

  for (u32 y = 0; y < height; y++) {
    for (u32 x = 0; x < width; x++) {
      const glm::vec4 coord = point(static_cast<f32>(x), static_cast<f32>(y));

      const glm::vec4 uv = bitmap_to_tex * coord;

      const f32 dist =
          (uv.x - 0.5f) * (uv.x - 0.5f) + (uv.y - 0.5f) * (uv.y - 0.5f);

      const u8 u_val = static_cast<u8>(100.f + 155.f * uv.x);
      const u8 v_val = static_cast<u8>(100.f + 155.f * uv.y);

      Color px_color{0, 255, 0};

      if (dist > 0.16f) {
        px_color = {u_val, 0, v_val};
      }

      if (dist < 0.09f) {
        px_color = {v_val, 0, u_val};
      }

      bmp_data.push_back(px_color.r);
      bmp_data.push_back(px_color.g);
      bmp_data.push_back(px_color.b);
    }
  }
}

Bitmap::Bitmap(const char *bmp_file)
    : bmp_data({}), bmp_width{0}, bmp_height(0) {
  std::ifstream file{
      bmp_file,
      std::ios_base::binary | std::ios_base::in,
  };

  if (file.fail()) {
    throw std::runtime_error("Failed to load file");
  }

  u8 buffer[header::SIZE]{0};

  file.read(reinterpret_cast<char *>(buffer), header::SIZE);

  const char BM[2] = {'B', 'M'};

  if (header::get<header::file_type>(buffer) !=
      *reinterpret_cast<const u16 *>(BM)) {
    throw std::runtime_error("Unsupported bitmap format");
  }

  if (header::get<header::bit_planes>(buffer) != 1) {
    throw std::runtime_error("Unsupported bitmap format bitplane number");
  }

  if (header::get<header::compression>(buffer) != 0) {
    throw std::runtime_error("Unsupported bitmap format compresion");
  }

  if (header::get<header::bits_per_pixel>(buffer) != 8 * sizeof(Color)) {
    throw std::runtime_error("Unsupported color per bits");
  }

  const i32 width{header::get<header::width>(buffer)};
  const i32 height{header::get<header::height>(buffer)};

  const bool top_down = height < 0;

  bmp_width = std::abs(width);
  bmp_height = std::abs(height);

  bmp_stride = computeStride(bmp_width);

  const u32 data_offset{header::get<header::bitmap_offset>(buffer)};
  u32 data_size{header::get<header::size_of_bitmap>(buffer)};

  if (data_size == 0) {
    data_size = bmp_height * bmp_stride;
  }

  u8 *const data = new u8[data_size];

  file.seekg(data_offset, std::ios_base::beg);
  file.read(reinterpret_cast<char *>(data), data_size);

  bmp_data.reserve(bmp_stride * bmp_height);

  for (usize y = 0; y < bmp_height; y++) {
    const usize row_index = bmp_stride * (top_down ? (bmp_height - 1 - y) : y);

    for (usize x = 0; x < bmp_width; x++) {
      const usize index = row_index + COLOR_BYTES * x;
      bmp_data.push_back(data[index]);
      bmp_data.push_back(data[index + 1]);
      bmp_data.push_back(data[index + 2]);
    }
  }

  delete[] data;
}

u32 Bitmap::offset(const i32 i, const i32 j) const {
  if (i < 0 or i >= (i32)bmp_width or j < 0 or j >= (i32)bmp_height) {
    throw std::out_of_range("Out of range");
  }
  return j * bmp_stride + i * COLOR_BYTES;
}

u32 computeStride(const u32 width) {
  const u32 minimum_width = width * COLOR_BYTES;

  return (minimum_width % BYTE_BOUNDRY)
             ? minimum_width + BYTE_BOUNDRY - minimum_width % BYTE_BOUNDRY
             : minimum_width;
}

void reverseRGB(Bitmap &bitmap) {
  u8 *const data = bitmap.data();

  for (u32 x = 0; x < bitmap.width(); x++) {
    for (u32 y = 0; y < bitmap.height(); y++) {
      const u32 offset =
          bitmap.offset(static_cast<i32>(x), static_cast<i32>(y));

      std::swap(data[offset], data[offset + 2]);
    }
  }
}

glm::mat4 bitmapToTextureTransform(const Bitmap &bitmap) {
  const f32 inv_width = 1.f / static_cast<f32>(bitmap.width());
  const f32 inv_height = 1.f / static_cast<f32>(bitmap.height());

  return {
      {inv_width, 0.f, 0.f, 0.f},
      {0.f, inv_height, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
      {0.5f * inv_width, 0.5f * inv_height, 0.f, 1.f},
  };
}

glm::mat4 textureToBitmapTransform(const Bitmap &bitmap) {
  const f32 width = static_cast<f32>(bitmap.width());
  const f32 height = static_cast<f32>(bitmap.height());

  return {
      {width, 0.f, 0.f, 0.f},
      {0.f, height, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
      {-0.5f, -0.5f, 0.f, 1.f},
  };
}

static f32 fract(const f32 value) { return std::fmod(value, 1.f); }

glm::vec3 getColor(const Bitmap &bitmap, f32 u, f32 v) {
  u = fract(fract(u) + 1.f);
  v = fract(fract(v) + 1.f);

  assert(u >= 0.f and u < 1.f);
  assert(v >= 0.f and v < 1.f);

  const glm::vec4 bitmap_coord =
      textureToBitmapTransform(bitmap) * cs200::point(u, v);

  const u32 offset = bitmap.offset(static_cast<i32>(bitmap_coord.x),
                                   static_cast<i32>(bitmap_coord.y));

  const Color &color = *reinterpret_cast<const Color *>(&bitmap.data()[offset]);

  return {
      static_cast<f32>(color.r),
      static_cast<f32>(color.g),
      static_cast<f32>(color.b),
  };
}

} // namespace cs200
