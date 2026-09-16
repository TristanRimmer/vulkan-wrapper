#include "descriptor_container.hh"
#include "descriptor_informer.hh"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <vulkan/vulkan_raii.hpp>

auto DescriptorUtils::DescriptorContainer::descriptor_pool()
    -> vk::raii::DescriptorPool & {
  return pool;
}
auto DescriptorUtils::DescriptorContainer::descriptor_set(std::size_t index)
    -> vk::raii::DescriptorSet & {
  return sets[index];
}
auto DescriptorUtils::DescriptorContainer::descriptor_set_layout()
    -> vk::raii::DescriptorSetLayout & {
  return set_layout;
}

auto DescriptorUtils::DescriptorContainer::create(
    std::size_t num_frames_in_flight,
    const DescriptorUtils::DescriptorInformer &descriptor_informer,
    const vk::raii::Device &device, const vk::raii::PhysicalDevice &phys_device)
    -> std::expected<DescriptorContainer, std::string> {
  /*
   * ============= Descriptor Pool ================
   */
  auto binding_orders = std::vector<DescriptorUtils::DescriptorBindingOptions>{
      descriptor_informer.get_binding_order()};

  // Populate here
  auto pool_sizes = std::vector<vk::DescriptorPoolSize>{};

  for (auto i = 0; i < binding_orders.size(); i++) {
    using Enum = DescriptorUtils::DescriptorBindingOptions;

    auto chosen_binding = vk::DescriptorType{};
    switch (binding_orders[i]) {
    case Enum::Uniform_Buffer: {
      chosen_binding = vk::DescriptorType::eUniformBuffer;
      break;
    };
    case Enum::Combined_Image_Sampler: {
      chosen_binding = vk::DescriptorType::eCombinedImageSampler;
      break;
    }
    case DescriptorBindingOptions::Sampler:
      chosen_binding = vk::DescriptorType::eSampler;
      break;
    }

    pool_sizes.emplace_back(vk::DescriptorPoolSize{
        .type = chosen_binding,
        .descriptorCount = static_cast<uint32_t>(num_frames_in_flight)});
  }

  auto pool_create_info = vk::DescriptorPoolCreateInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = static_cast<uint32_t>(num_frames_in_flight),
      .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
      .pPoolSizes = pool_sizes.data()};

  auto descriptor_pool = vk::raii::DescriptorPool(device, pool_create_info);

  /*
   * ============= Descriptor Set Layouts ================
   */
  auto bindings = descriptor_informer.get_set_bindings();

  std::cout << "Warn: In the event of errors, it might be because this is a "
               "refernce. Perhaps needs to be a copy"
            << std::endl;

  auto desciptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo{
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  auto base_descriptor_set_layout =
      vk::raii::DescriptorSetLayout(device, desciptor_set_layout_create_info);

  auto descriptor_set_layouts_vec = std::vector<vk::DescriptorSetLayout>(
      num_frames_in_flight, *base_descriptor_set_layout);

  /*
   * ============= Descriptor Sets ================
   */
  auto descriptor_set_alloc_info = vk::DescriptorSetAllocateInfo{
      .descriptorPool = descriptor_pool,
      .descriptorSetCount =
          static_cast<uint32_t>(descriptor_set_layouts_vec.size()),
      .pSetLayouts = descriptor_set_layouts_vec.data()};

  auto descriptor_sets = std::vector<vk::raii::DescriptorSet>{
      device.allocateDescriptorSets(descriptor_set_alloc_info)};

  for (auto fif = 0; fif < num_frames_in_flight; fif++) {
    // INFO: Initially these were created as single lvalues in the binding loop
    // but i dont think it had the correct lifetime, as the writes refernce them
    // despite having a longer liftime
    auto buffer_infos = std::vector<vk::DescriptorBufferInfo>();
    auto sampler_and_or_image_infos = std::vector<vk::DescriptorImageInfo>();
    auto descriptor_writes = std::vector<vk::WriteDescriptorSet>{};
    // That is why the sampler_and_or_image_infos has a size of them joined
    buffer_infos.reserve(descriptor_informer.get_num_ubos());
    sampler_and_or_image_infos.reserve(
        descriptor_informer.get_num_combined_image_samplers() +
        descriptor_informer.get_num_samplers());
    descriptor_writes.reserve(binding_orders.size());

    // Per frame in flight, bind the resources to the descriptors and
    // descriptor sets
    for (auto binding_i = 0; binding_i < binding_orders.size(); binding_i++) {
      using Enum = DescriptorUtils::DescriptorBindingOptions;

      auto current_write = vk::WriteDescriptorSet{
          .dstSet = descriptor_sets[fif],
          .dstBinding = static_cast<uint32_t>(binding_i),
          .dstArrayElement = 0,
          .descriptorCount = 1,
      };

      switch (binding_orders[binding_i]) {
      case Enum::Uniform_Buffer: {
        buffer_infos.emplace_back(vk::DescriptorBufferInfo{
            .buffer = descriptor_informer.get_ubo_buffers_ref(binding_i)[fif],
            .offset = 0,
            .range = descriptor_informer.get_ubo_buffer_size(binding_i)});

        current_write.descriptorType = vk::DescriptorType::eUniformBuffer;
        current_write.pBufferInfo = &buffer_infos.back();
        break;
      };
      case Enum::Combined_Image_Sampler: {
        sampler_and_or_image_infos.emplace_back(vk::DescriptorImageInfo{
            .sampler =
                descriptor_informer.get_combined_image_sampler_ref(binding_i)
                    .first.get(),
            .imageView =
                descriptor_informer.get_combined_image_sampler_ref(binding_i)
                    .second.get(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal});
        current_write.descriptorType =
            vk::DescriptorType::eCombinedImageSampler;
        current_write.pImageInfo = &sampler_and_or_image_infos.back();
        break;
      }
      case Enum::Sampler:
        sampler_and_or_image_infos.emplace_back(vk::DescriptorImageInfo{
            .sampler = descriptor_informer.get_sampler_ref(binding_i)});
        current_write.descriptorType = vk::DescriptorType::eSampler;
        current_write.pImageInfo = &sampler_and_or_image_infos.back();
        break;
      }
      descriptor_writes.push_back(current_write);
    }

    device.updateDescriptorSets(descriptor_writes, {});
  }

  auto object = DescriptorContainer(std::move(base_descriptor_set_layout),
                                    std::move(descriptor_pool),
                                    std::move(descriptor_sets));

  return object;
}
