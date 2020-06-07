#include "pch.h"
#include "Rendering/Model.h"

namespace Firefly
{
	Model::Model(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) :
		m_mesh(mesh),
		m_material(material),
		m_modelMatrix(glm::mat4(1))
	{
	}

	Model::~Model()
	{
	}

	void Model::Bind()
	{
		m_material->Bind();
		GetShader()->SetUniformMatrix4("u_modelMat", m_modelMatrix);
		GetShader()->SetUniformMatrix3("u_normalMat", glm::transpose(glm::inverse(glm::mat3(m_modelMatrix))));
		m_mesh->Bind();
	}

	void Model::SetMaterial(std::shared_ptr<Material> material)
	{
		m_material = material;
	}

	void Model::SetModelMatrix(const glm::mat4& modelMatrix)
	{
		m_modelMatrix = modelMatrix;
	}

	std::shared_ptr<Shader> Model::GetShader()
	{
		return m_material->GetShader();
	}

	std::shared_ptr<Mesh> Model::GetMesh()
	{
		return m_mesh;
	}
}