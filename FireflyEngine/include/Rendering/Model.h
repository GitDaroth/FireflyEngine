#pragma once

#include "Rendering/Mesh.h"
#include "Rendering/Material.h"

namespace Firefly
{
	class Model
	{
	public:
		Model(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
		~Model();

		void Bind();

		void SetModelMatrix(const glm::mat4& modelMatrix);

		std::shared_ptr<Shader> GetShader();
		std::shared_ptr<Mesh> GetMesh();

	private:
		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Material> m_material;
		glm::mat4 m_modelMatrix;
	};
}