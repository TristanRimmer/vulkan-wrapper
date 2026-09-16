#ifndef VULKAN_WRAPPER_DESCRIPTOR_DESCRIPTOR_INFORMER_HH
#define VULKAN_WRAPPER_DESCRIPTOR_DESCRIPTOR_INFORMER_HH

#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <functional>
#include <vulkan/vulkan_raii.hpp>
namespace DescriptorUtils {

enum class DescriptorBindingOptions {
  Uniform_Buffer,
  Sampler,
  Combined_Image_Sampler,
};

class DescriptorInformer {
public:
  DescriptorInformer() : set_bindings({}) {}

  auto register_uniform_buffer(vk::ShaderStageFlagBits stage_flags,
                               const std::vector<vk::raii::Buffer> &buffer_refs,
                               std::size_t buffer_size) -> uint32_t;
  auto register_sampler(vk::ShaderStageFlagBits stage_flags,
                        const vk::raii::Sampler &sampler) -> uint32_t;
  auto register_combined_image_sampler(vk::ShaderStageFlagBits stage_flags,
                                       const vk::raii::Sampler &sampler,
                                       const vk::raii::ImageView &image_view)
      -> uint32_t;

  auto get_set_bindings() const
      -> const std::vector<vk::DescriptorSetLayoutBinding> &;

  auto get_ubo_buffers_ref(std::size_t buffer_index) const
      -> const std::vector<vk::raii::Buffer> &;
  auto get_ubo_buffer_size(std::size_t buffer_index) const -> std::size_t;

  auto get_sampler_ref(std::size_t sampler_index) const
      -> const vk::raii::Sampler &;

  auto get_combined_image_sampler_ref(std::size_t comb_index) const
      -> std::pair<std::reference_wrapper<const vk::raii::Sampler>,
                   std::reference_wrapper<const vk::raii::ImageView>>;

  auto get_binding_order() const
      -> const std::vector<DescriptorUtils::DescriptorBindingOptions> &;

  auto get_num_ubos() const -> std::size_t;
  auto get_num_combined_image_samplers() const -> std::size_t;
  auto get_num_samplers() const -> std::size_t;

private:
  std::vector<DescriptorUtils::DescriptorBindingOptions> binding_options;

  std::vector<std::pair<
      std::reference_wrapper<const std::vector<vk::raii::Buffer>>, std::size_t>>
      buffer_and_size_refs;
  std::vector<std::reference_wrapper<const vk::raii::Sampler>> samplers;
  std::vector<std::pair<std::reference_wrapper<const vk::raii::Sampler>,
                        std::reference_wrapper<const vk::raii::ImageView>>>
      combined_image_samplers;

  std::vector<vk::DescriptorSetLayoutBinding> set_bindings;
};

} // namespace DescriptorUtils
#endif
