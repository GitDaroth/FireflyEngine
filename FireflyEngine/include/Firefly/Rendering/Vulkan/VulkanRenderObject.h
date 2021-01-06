#pragma once

#include "Rendering/Vulkan/VulkanMaterial.h"
#include "Rendering/Vulkan/VulkanMesh.h"

namespace Firefly
{
	class VulkanRenderObject
	{
	public:
		VulkanRenderObject(VulkanMesh* mesh, VulkanMaterial* material);

		void Bind(vk::CommandBuffer commandBuffer, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

		void SetModelMatrix(glm::mat4 modelMatrix);

	private:
		VulkanMaterial* m_material;
		VulkanMesh* m_mesh;
		glm::mat4 m_modelMatrix;
	};
}