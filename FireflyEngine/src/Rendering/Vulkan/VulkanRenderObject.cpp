#include "pch.h"
#include "Rendering/Vulkan/VulkanRenderObject.h"

namespace Firefly
{
	VulkanRenderObject::VulkanRenderObject(VulkanMesh* mesh, VulkanMaterial* material) :
		m_mesh(mesh),
		m_material(material),
		m_modelMatrix(glm::mat4(1.0f))
	{
	}

	void VulkanRenderObject::Draw(vk::CommandBuffer commandBuffer, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
	{
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_material->GetPipeline());

		glm::mat4 matrix = projectionMatrix * viewMatrix * m_modelMatrix;
		commandBuffer.pushConstants(m_material->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &matrix);

		m_mesh->Bind(commandBuffer);

		commandBuffer.drawIndexed(m_mesh->GetIndexCount(), 1, 0, 0, 0);
	}

	void VulkanRenderObject::SetModelMatrix(glm::mat4 modelMatrix)
	{
		m_modelMatrix = modelMatrix;
	}
}