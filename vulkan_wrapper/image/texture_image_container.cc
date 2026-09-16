#include "texture_image_container.hh"
#include "stb_image.h"
#include "vulkan_wrapper/device/helpers.hh"
#include "vulkan_wrapper/image/image_utils.hh"

auto ImageUtils::TextureImageContainer::create(
    ImageUtils::TextureImageCreateInfo info, const vk::raii::Device &logical,
    const vk::raii::PhysicalDevice &physical_device,
    const vk::raii::CommandPool &command_pool, const vk::raii::Queue &queue)
    -> std::expected<TextureImageContainer, std::string> {
  auto tex_width = int{};
  auto tex_height = int{};
  auto tex_channels = int{};

  auto *pixels = stbi_load(info.image_path, &tex_width, &tex_height,
                           &tex_channels, STBI_rgb_alpha);

  auto image_size_on_device =
      static_cast<vk::DeviceSize>(tex_width * tex_height * 4);

  if (!pixels)
    return std::unexpected("TextureImageContainer Init STBI Image Load Error");

  auto maybe_staging_buff_data =
      DeviceUtil::create_buffer(logical, physical_device, image_size_on_device,
                                vk::BufferUsageFlagBits::eTransferSrc,
                                vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent);

  if (!maybe_staging_buff_data)
    return std::unexpected("TextureImageContainer Staging Buffer init error: " +
                           maybe_staging_buff_data.error());

  auto [staging_buffer, staging_buffer_mem] =
      std::move(maybe_staging_buff_data.value());

  // TODO: An implementation help fucntion for a host visible GPU buffer with an
  // 'upload data to gpu' function

  void *raw_mem = staging_buffer_mem.mapMemory(0, image_size_on_device);
  memcpy(raw_mem, pixels, image_size_on_device);
  staging_buffer_mem.unmapMemory();

  stbi_image_free(pixels);

  auto image = vk::raii::Image{nullptr};
  auto image_memory = vk::raii::DeviceMemory{nullptr};

  auto maybe_image_and_memory = ImageUtils::create_image(
      logical, physical_device, {tex_width, tex_height},
      vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  if (!maybe_image_and_memory)
    return std::unexpected(
        "TextureImageContainer Image and Image Memory Init Error: " +
        maybe_image_and_memory.error());

  std::tie(image, image_memory) = std::move(maybe_image_and_memory.value());

  // Spin a temporary command buffer for buffer copy
  auto one_time_use_command_buffer_alloc_info =
      vk::CommandBufferAllocateInfo{.commandPool = command_pool,
                                    .level = vk::CommandBufferLevel::ePrimary,
                                    .commandBufferCount = 1};

  auto one_time_use_command_buffer = std::move(
      logical.allocateCommandBuffers(one_time_use_command_buffer_alloc_info)
          .front());

  one_time_use_command_buffer.begin(
      {.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  // TODO: Transition Layouts are still quite confusing. Learn!

  ImageUtils::transition_image_layout(one_time_use_command_buffer, image,
                                      vk::ImageLayout::eUndefined,
                                      vk::ImageLayout::eTransferDstOptimal);
  ImageUtils::copy_buffer_to_image(one_time_use_command_buffer, staging_buffer,
                                   image, tex_width, tex_height);
  ImageUtils::transition_image_layout(one_time_use_command_buffer, image,
                                      vk::ImageLayout::eTransferDstOptimal,
                                      vk::ImageLayout::eShaderReadOnlyOptimal);

  one_time_use_command_buffer.end();

  queue.submit(vk::SubmitInfo{.commandBufferCount = 1,
                              .pCommandBuffers = &*one_time_use_command_buffer},
               nullptr);
  queue.waitIdle();

  // Finally, create the image view
  auto image_view_create_info = vk::ImageViewCreateInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  auto image_view = vk::raii::ImageView(logical, image_view_create_info);

  auto object = TextureImageContainer(std::move(image), std::move(image_memory),
                                      std::move(image_view));

  return object;
}
