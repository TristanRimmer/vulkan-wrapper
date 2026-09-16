#include "image_utils.hh"
#include "vulkan/vulkan.hpp"
#include "vulkan_wrapper/device/helpers.hh"

auto ImageUtils::create_image(const vk::raii::Device &device,
                              const vk::raii::PhysicalDevice &physical,
                              std::pair<uint32_t, uint32_t> image_dimensions,
                              vk::Format image_format, vk::ImageTiling tiling,
                              vk::ImageUsageFlags usage,
                              vk::MemoryPropertyFlags properties)
    -> std::expected<std::pair<vk::raii::Image, vk::raii::DeviceMemory>,
                     std::string> {
  auto image_create_info = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = image_format,
      .extent = {image_dimensions.first, image_dimensions.second, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive};

  auto image = vk::raii::Image(device, image_create_info);

  auto memory_requirements =
      vk::MemoryRequirements{image.getMemoryRequirements()};

  auto maybe_memory_valid = DeviceUtil::find_memory_type(
      physical, memory_requirements.memoryTypeBits, properties);

  if (!maybe_memory_valid)
    return std::unexpected("Image Creation Error: " +
                           maybe_memory_valid.error());

  auto allocation_info = vk::MemoryAllocateInfo{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = maybe_memory_valid.value(),
  };

  auto image_memory = vk::raii::DeviceMemory(device, allocation_info);

  image.bindMemory(image_memory, 0);

  return std::pair{std::move(image), std::move(image_memory)};
}

auto ImageUtils::transition_image_layout(
    vk::raii::CommandBuffer &command_buffer, const vk::raii::Image &image,
    vk::ImageLayout old_layout, vk::ImageLayout new_layout) -> void {
  auto barrier = vk::ImageMemoryBarrier{
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
      .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
      .image = image,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .levelCount = 1,
                           .layerCount = 1}};

  auto source_stage = vk::PipelineStageFlags{};
  auto dest_stage = vk::PipelineStageFlags{};

  if (old_layout == vk::ImageLayout::eUndefined and
      new_layout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dest_stage = vk::PipelineStageFlagBits::eTransfer;
  } else if (old_layout == vk::ImageLayout::eTransferDstOptimal and
             new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    source_stage = vk::PipelineStageFlagBits::eTransfer;
    dest_stage = vk::PipelineStageFlagBits::eFragmentShader;
  }
  // WARN: This should have an else and it should return a std::unexpected

  command_buffer.pipelineBarrier(source_stage, dest_stage, {}, {}, nullptr,
                                 barrier);
}

auto ImageUtils::copy_buffer_to_image(vk::raii::CommandBuffer &command_buffer,
                                      const vk::raii::Buffer &buffer,
                                      vk::raii::Image &image, uint32_t width,
                                      uint32_t height) -> void {
  auto region = vk::BufferImageCopy{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .mipLevel = 0,
                           .baseArrayLayer = 0,
                           .layerCount = 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1}};

  command_buffer.copyBufferToImage(
      buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}
