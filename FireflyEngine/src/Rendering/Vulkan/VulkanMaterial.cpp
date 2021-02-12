#include "pch.h"
#include "Rendering/Vulkan/VulkanMaterial.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanTexture.h"

namespace Firefly
{
    VulkanMaterial::VulkanMaterial() :
        Material()
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        m_device = vkContext->GetDevice()->GetHandle();
        m_descriptorPool = vkContext->GetDescriptorPool();
    }

    void VulkanMaterial::Destroy()
    {
        m_device.destroyDescriptorSetLayout(m_materialTexturesDescriptorSetLayout);
    }

    vk::DescriptorSet VulkanMaterial::GetTexturesDescriptorSet() const
    {
        return m_materialTexturesDescriptorSet;
    }

    void VulkanMaterial::OnInit()
    {
        vk::DescriptorSetLayoutBinding albedoTextureLayoutBinding{};
        albedoTextureLayoutBinding.binding = 0;
        albedoTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        albedoTextureLayoutBinding.descriptorCount = 1;
        albedoTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        albedoTextureLayoutBinding.pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBinding normalTextureLayoutBinding{};
        normalTextureLayoutBinding.binding = 1;
        normalTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        normalTextureLayoutBinding.descriptorCount = 1;
        normalTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        normalTextureLayoutBinding.pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBinding roughnessTextureLayoutBinding{};
        roughnessTextureLayoutBinding.binding = 2;
        roughnessTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        roughnessTextureLayoutBinding.descriptorCount = 1;
        roughnessTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        roughnessTextureLayoutBinding.pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBinding metalnessTextureLayoutBinding{};
        metalnessTextureLayoutBinding.binding = 3;
        metalnessTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        metalnessTextureLayoutBinding.descriptorCount = 1;
        metalnessTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        metalnessTextureLayoutBinding.pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBinding occlusionTextureLayoutBinding{};
        occlusionTextureLayoutBinding.binding = 4;
        occlusionTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        occlusionTextureLayoutBinding.descriptorCount = 1;
        occlusionTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        occlusionTextureLayoutBinding.pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBinding heightTextureLayoutBinding{};
        heightTextureLayoutBinding.binding = 5;
        heightTextureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        heightTextureLayoutBinding.descriptorCount = 1;
        heightTextureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
        heightTextureLayoutBinding.pImmutableSamplers = nullptr;

        std::vector<vk::DescriptorSetLayoutBinding> bindings =
        {
            albedoTextureLayoutBinding,
            normalTextureLayoutBinding,
            roughnessTextureLayoutBinding,
            metalnessTextureLayoutBinding,
            occlusionTextureLayoutBinding,
            heightTextureLayoutBinding
        };

        // PartiallyBound: (PhysicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound needs to be enabled)
        // -> Allows to update only part of the combined image sampler descriptors with actual data 
        // UpdateAfterBind: (PhysicalDeviceDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind needs to be enabled)
        // -> Allows to update a combined image sampler descriptor on the fly -> corres. flag needs to be set in DescriptorSetLayoutCreateInfo and DescriptorPoolCreateInfo
        std::vector<vk::DescriptorBindingFlags> bindingFlags(bindings.size(), vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
        vk::DescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCreateInfo{};
        layoutBindingFlagsCreateInfo.pNext = nullptr;
        layoutBindingFlagsCreateInfo.bindingCount = bindingFlags.size();
        layoutBindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

        vk::DescriptorSetLayoutCreateInfo materialTexturesDescriptorSetLayoutCreateInfo{};
        materialTexturesDescriptorSetLayoutCreateInfo.pNext = &layoutBindingFlagsCreateInfo;
        materialTexturesDescriptorSetLayoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool; // Needed in order to update textures on the fly
        materialTexturesDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        materialTexturesDescriptorSetLayoutCreateInfo.pBindings = bindings.data();
        vk::Result result = m_device.createDescriptorSetLayout(&materialTexturesDescriptorSetLayoutCreateInfo, nullptr, &m_materialTexturesDescriptorSetLayout);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.pNext = nullptr;
        descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &m_materialTexturesDescriptorSetLayout;
        result = m_device.allocateDescriptorSets(&descriptorSetAllocateInfo, &m_materialTexturesDescriptorSet);
        FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");
    }

    void VulkanMaterial::OnSetTexture(std::shared_ptr<Texture> texture, TextureUsage usage)
    {
        uint32_t binding = 0;
        switch (usage)
        {
        case TextureUsage::Albedo:
            binding = 0;
            break;
        case TextureUsage::Normal:
            binding = 1;
            break;
        case TextureUsage::Roughness:
            binding = 2;
            break;
        case TextureUsage::Metalness:
            binding = 3;
            break;
        case TextureUsage::Occlusion:
            binding = 4;
            break;
        case TextureUsage::Height:
            binding = 5;
            break;
        }

        std::shared_ptr<VulkanTexture> vkTexture = std::dynamic_pointer_cast<VulkanTexture>(texture);

        vk::DescriptorImageInfo descriptorImageInfo{};
        descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        descriptorImageInfo.imageView = vkTexture->GetImageView();
        descriptorImageInfo.sampler = vkTexture->GetSampler();

        vk::WriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.dstSet = m_materialTexturesDescriptorSet;
        writeDescriptorSet.dstBinding = binding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = nullptr;
        writeDescriptorSet.pImageInfo = &descriptorImageInfo;
        writeDescriptorSet.pTexelBufferView = nullptr;

        m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
    }
}