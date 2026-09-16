#ifndef VULKAN_WRAPPER_UNIFORM_BUFFER_CONTAINER_HH
#define VULKAN_WRAPPER_UNIFORM_BUFFER_CONTAINER_HH

#include "vulkan_wrapper/device/helpers.hh"
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

namespace BufferUtils {

struct UniformBufferContainerOnlyCreateInfo {
  vk::ShaderStageFlagBits stage;
  std::size_t max_frames_in_flight;
};

template <typename UB> class UniformBufferContainerOnly {
public:
  UniformBufferContainerOnly() = delete;

  static auto create(UniformBufferContainerOnlyCreateInfo info,
                     const vk::raii::Device &device,
                     const vk::raii::PhysicalDevice &phys_device)
      -> std::expected<UniformBufferContainerOnly, std::string>;

  auto uniform_buffer(std::size_t index) -> vk::raii::Buffer &;
  auto uniform_buffer_memory(std::size_t index) -> vk::raii::DeviceMemory &;
  auto uniform_buffer_mapped(std::size_t index) -> void *;

  auto all_uniform_buffers() -> const std::vector<vk::raii::Buffer> &;

private:
  UniformBufferContainerOnly(std::vector<vk::raii::Buffer> &&v_u_b,
                             std::vector<vk::raii::DeviceMemory> &&v_u_b_m,
                             std::vector<void *> v_u_b_r_m)
      : _uniform_buffers(std::move(v_u_b)),
        _uniform_buffers_memory(std::move(v_u_b_m)),
        _uniform_buffers_mapped(std::move(v_u_b_r_m)) {}

private:
  std::vector<vk::raii::Buffer> _uniform_buffers;
  std::vector<vk::raii::DeviceMemory> _uniform_buffers_memory;
  std::vector<void *> _uniform_buffers_mapped;
};

} // namespace BufferUtils

template <typename UB>
auto BufferUtils::UniformBufferContainerOnly<UB>::all_uniform_buffers()
    -> const std::vector<vk::raii::Buffer> & {
  return _uniform_buffers;
}
template <typename UB>
auto BufferUtils::UniformBufferContainerOnly<UB>::create(
    BufferUtils::UniformBufferContainerOnlyCreateInfo info,
    const vk::raii::Device &device, const vk::raii::PhysicalDevice &phys_device)
    -> std::expected<UniformBufferContainerOnly, std::string> {

  // Initialise Uniform Buffers
  auto uniform_buffers = std::vector<vk::raii::Buffer>{};
  auto uniform_buffers_memory = std::vector<vk::raii::DeviceMemory>{};
  auto uniform_buffers_mapped = std::vector<void *>{};

  for (auto i = 0; i < info.max_frames_in_flight; i++) {
    auto buffer_size = vk::DeviceSize{sizeof(UB)};
    auto maybe_buf_and_mem = DeviceUtil::create_buffer(
        device, phys_device, buffer_size,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent);

    if (!maybe_buf_and_mem)
      return std::unexpected("3D App Uniform Buffer Init Error: " +
                             maybe_buf_and_mem.error());
    auto [buffer, buffer_mem] = std::move(maybe_buf_and_mem.value());

    uniform_buffers.emplace_back(std::move(buffer));
    uniform_buffers_memory.emplace_back(std::move(buffer_mem));
    uniform_buffers_mapped.emplace_back(
        uniform_buffers_memory.back().mapMemory(0, buffer_size));
  }

  auto object = UniformBufferContainerOnly<UB>(
      std::move(uniform_buffers), std::move(uniform_buffers_memory),
      std::move(uniform_buffers_mapped));

  return object;
}

template <typename UB>
auto BufferUtils::UniformBufferContainerOnly<UB>::uniform_buffer(
    std::size_t index) -> vk::raii::Buffer & {
  return _uniform_buffers[index];
}

template <typename UB>
auto BufferUtils::UniformBufferContainerOnly<UB>::uniform_buffer_memory(
    std::size_t index) -> vk::raii::DeviceMemory & {
  return _uniform_buffers_memory[index];
}

template <typename UB>
auto BufferUtils::UniformBufferContainerOnly<UB>::uniform_buffer_mapped(
    std::size_t index) -> void * {
  return _uniform_buffers_mapped[index];
}

#endif
