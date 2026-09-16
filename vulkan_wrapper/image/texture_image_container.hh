#ifndef VULKAN_WRAPPER_TEXTURE_IMAGE_CONTAINER_HH
#define VULKAN_WRAPPER_TEXTURE_IMAGE_CONTAINER_HH

#include <expected>
#include <vulkan/vulkan_raii.hpp>
namespace ImageUtils {

struct TextureImageCreateInfo {
  const char *image_path;
};

class TextureImageContainer {
public:
  TextureImageContainer() = delete;

  static auto create(ImageUtils::TextureImageCreateInfo info,
                     const vk::raii::Device &logical,
                     const vk::raii::PhysicalDevice &physical_device,
                     const vk::raii::CommandPool &command_pool,
                     const vk::raii::Queue &queue)

      -> std::expected<TextureImageContainer, std::string>;

  auto image() -> vk::raii::Image &;
  auto memory() -> vk::raii::DeviceMemory &;
  auto view() -> vk::raii::ImageView &;

private:
  TextureImageContainer(vk::raii::Image &&d_i, vk::raii::DeviceMemory &&i_m,
                        vk::raii::ImageView &&i_v)
      : tex_image(std::move(d_i)), image_memory(std::move(i_m)),
        image_view(std::move(i_v)) {}

private:
  vk::raii::Image tex_image;
  vk::raii::DeviceMemory image_memory;
  vk::raii::ImageView image_view;
};

} // namespace ImageUtils

#endif
