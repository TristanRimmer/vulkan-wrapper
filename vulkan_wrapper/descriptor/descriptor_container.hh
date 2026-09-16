#ifndef VULKAN_WRAPPER_DESCRIPTOR_CONTAINER_HH
#define VULKAN_WRAPPER_DESCRIPTOR_CONTAINER_HH

#include "descriptor_informer.hh"
#include <expected>
#include <vulkan/vulkan_raii.hpp>

namespace DescriptorUtils {

class DescriptorContainer {
public:
  DescriptorContainer() = delete;

  auto descriptor_set_layout() -> vk::raii::DescriptorSetLayout &;
  auto descriptor_pool() -> vk::raii::DescriptorPool &;
  auto descriptor_set(std::size_t index) -> vk::raii::DescriptorSet &;

  static auto
  create(std::size_t num_frames_in_flight,
         const DescriptorUtils::DescriptorInformer &descriptor_informer,
         const vk::raii::Device &device,
         const vk::raii::PhysicalDevice &phys_device)
      -> std::expected<DescriptorContainer, std::string>;

private:
  DescriptorContainer(vk::raii::DescriptorSetLayout &&s_l,
                      vk::raii::DescriptorPool &&d_p,
                      std::vector<vk::raii::DescriptorSet> &&d_s)
      : set_layout(std::move(s_l)), pool(std::move(d_p)), sets(std::move(d_s)) {
  }

private:
  vk::raii::DescriptorSetLayout set_layout;
  vk::raii::DescriptorPool pool;
  std::vector<vk::raii::DescriptorSet> sets;
};
} // namespace DescriptorUtils

#endif
