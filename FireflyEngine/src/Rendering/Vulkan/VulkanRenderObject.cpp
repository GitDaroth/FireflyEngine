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

	VulkanMaterial* VulkanRenderObject::GetMaterial() const
	{
		return m_material;
	}

	VulkanMesh* VulkanRenderObject::GetMesh() const
	{
		return m_mesh;
	}

	glm::mat4 VulkanRenderObject::GetModelMatrix() const
	{
		return m_modelMatrix;
	}

	void VulkanRenderObject::SetModelMatrix(glm::mat4 modelMatrix)
	{
		m_modelMatrix = modelMatrix;
	}
}