#pragma once

#include "Rendering/Vulkan/VulkanMaterial.h"
#include "Rendering/Vulkan/VulkanMesh.h"

namespace Firefly
{
	class VulkanRenderObject
	{
	public:
		VulkanRenderObject(VulkanMesh* mesh, VulkanMaterial* material);

		VulkanMaterial* GetMaterial() const;
		VulkanMesh* GetMesh() const;
		glm::mat4 GetModelMatrix() const;
		void SetModelMatrix(glm::mat4 modelMatrix);

	private:
		VulkanMaterial* m_material;
		VulkanMesh* m_mesh;
		glm::mat4 m_modelMatrix;
	};
}