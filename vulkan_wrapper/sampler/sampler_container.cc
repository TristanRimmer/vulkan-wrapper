#include "sampler_container.hh"
#include <vulkan/vulkan_raii.hpp>

auto SamplerUtils::SamplerContainer::sampler() -> vk::raii::Sampler & {
  return image_sampler;
}

auto SamplerUtils::SamplerContainer::create(vk::SamplerCreateInfo create_info,
                                            const vk::raii::Device &logical)
    -> std::expected<SamplerContainer, std::string> {
  auto object =
      SamplerContainer(std::move(vk::raii::Sampler(logical, create_info)));

  return object;
}
