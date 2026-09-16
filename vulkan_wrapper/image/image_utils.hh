#ifndef VULKAN_WRAPPER_IMAGE_UTILS_HH
#define VULKAN_WRAPPER_IMAGE_UTILS_HH

#include "vulkan/vulkan.hpp"
#include <expected>
#include <utility>
#include <vulkan/vulkan_raii.hpp>
namespace ImageUtils {
auto create_image(const vk::raii::Device &device,
                  const vk::raii::PhysicalDevice &physical,
                  std::pair<uint32_t, uint32_t> image_dimensions,
                  vk::Format image_format, vk::ImageTiling tiling,
                  vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
    -> std::expected<std::pair<vk::raii::Image, vk::raii::DeviceMemory>,
                     std::string>;

auto transition_image_layout(vk::raii::CommandBuffer &command_buffer,
                             const vk::raii::Image &image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout) -> void;

auto copy_buffer_to_image(vk::raii::CommandBuffer &command_buffer,
                          const vk::raii::Buffer &buffer,
                          vk::raii::Image &image, uint32_t width,
                          uint32_t height) -> void;
} // namespace ImageUtils

#endif
