#include "descriptor_informer.hh"
#include <utility>

auto DescriptorUtils::DescriptorInformer::register_uniform_buffer(
    vk::ShaderStageFlagBits stage_flags,
    const std::vector<vk::raii::Buffer> &buffer_refs, std::size_t buffer_size)
    -> uint32_t {
  auto binding_number = binding_options.size();

  binding_options.emplace_back(
      DescriptorUtils::DescriptorBindingOptions::Uniform_Buffer);

  set_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding = static_cast<uint32_t>(binding_number),
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags = stage_flags});

  buffer_and_size_refs.emplace_back(std::cref(buffer_refs), buffer_size);

  return binding_number;
}

auto DescriptorUtils::DescriptorInformer::register_sampler(
    vk::ShaderStageFlagBits stage_flags, const vk::raii::Sampler &sampler)
    -> uint32_t {
  auto binding_number = binding_options.size();

  binding_options.emplace_back(
      DescriptorUtils::DescriptorBindingOptions::Sampler);

  set_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding = static_cast<uint32_t>(binding_number),
      .descriptorType = vk::DescriptorType::eSampler,
      .descriptorCount = 1,
      .stageFlags = stage_flags});

  samplers.emplace_back(std::cref(sampler));

  return binding_number;
}

auto DescriptorUtils::DescriptorInformer::register_combined_image_sampler(
    vk::ShaderStageFlagBits stage_flags, const vk::raii::Sampler &sampler,
    const vk::raii::ImageView &image_view) -> uint32_t {
  auto binding_number = binding_options.size();

  binding_options.emplace_back(
      DescriptorUtils::DescriptorBindingOptions::Combined_Image_Sampler);

  set_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
      .binding = static_cast<uint32_t>(binding_number),
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 1,
      .stageFlags = stage_flags});

  combined_image_samplers.emplace_back(std::cref(sampler),
                                       std::cref(image_view));

  return binding_number;
}

auto DescriptorUtils::DescriptorInformer::get_set_bindings() const
    -> const std::vector<vk::DescriptorSetLayoutBinding> & {
  return set_bindings;
}

auto DescriptorUtils::DescriptorInformer::get_ubo_buffers_ref(
    std::size_t buffer_index) const -> const std::vector<vk::raii::Buffer> & {
  return buffer_and_size_refs[buffer_index].first.get();
}

auto DescriptorUtils::DescriptorInformer::get_ubo_buffer_size(
    std::size_t buffer_index) const -> std::size_t {
  return buffer_and_size_refs[buffer_index].second;
}

auto DescriptorUtils::DescriptorInformer::get_sampler_ref(
    std::size_t sampler_index) const -> const vk::raii::Sampler & {
  return samplers[sampler_index].get();
}

auto DescriptorUtils::DescriptorInformer::get_combined_image_sampler_ref(
    std::size_t comb_index) const
    -> std::pair<std::reference_wrapper<const vk::raii::Sampler>,
                 std::reference_wrapper<const vk::raii::ImageView>> {
  return combined_image_samplers[comb_index];
}

auto DescriptorUtils::DescriptorInformer::get_binding_order() const
    -> const std::vector<DescriptorUtils::DescriptorBindingOptions> & {
  return binding_options;
}

auto DescriptorUtils::DescriptorInformer::get_num_ubos() const -> std::size_t {
  return buffer_and_size_refs.size();
}

auto DescriptorUtils::DescriptorInformer::get_num_combined_image_samplers()
    const -> std::size_t {
  return combined_image_samplers.size();
}

auto DescriptorUtils::DescriptorInformer::get_num_samplers() const
    -> std::size_t {
  return samplers.size();
}
