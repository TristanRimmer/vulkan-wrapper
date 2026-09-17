#ifndef VULKAN_WRAPPER_SAMPLER_CONTAINER_HH
#define VULKAN_WRAPPER_SAMPLER_CONTAINER_HH

#include "vulkan/vulkan.hpp"
#include <expected>
#include <vulkan/vulkan_raii.hpp>
namespace SamplerUtils {

class SamplerContainer {
public:
  SamplerContainer() = delete;

  static auto create(vk::SamplerCreateInfo create_info,
                     const vk::raii::Device &logical)
      -> std::expected<SamplerContainer, std::string>;

  auto sampler() -> vk::raii::Sampler &;

private:
  SamplerContainer(vk::raii::Sampler &&s) : image_sampler(std::move(s)) {}

private:
  vk::raii::Sampler image_sampler;
};
} // namespace SamplerUtils

#endif
