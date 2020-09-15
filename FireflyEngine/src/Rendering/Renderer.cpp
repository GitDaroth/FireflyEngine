#include "pch.h"
#include "Rendering/Renderer.h"

#include "Rendering/RenderingAPI.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/MaterialComponent.h"
#include "Scene/Components/TransformComponent.h"

namespace Firefly
{
	Renderer::Renderer()
	{
		RenderingAPI::GetRenderFunctions()->Init();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::DrawScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera)
	{
		glm::mat4 viewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();

		auto view = scene->m_entityRegistry.view<Firefly::MeshComponent, Firefly::MaterialComponent, Firefly::TransformComponent>();
		for (auto entity : view)
		{
			auto mesh = view.get<Firefly::MeshComponent>(entity).m_mesh;
			auto material = view.get<Firefly::MaterialComponent>(entity).m_material;
			auto transform = view.get<Firefly::TransformComponent>(entity).m_transform;
			material->Bind();
			material->GetShader()->SetUniformMatrix4("u_modelMat", transform);
			material->GetShader()->SetUniformMatrix3("u_normalMat", glm::transpose(glm::inverse(glm::mat3(transform))));
			material->GetShader()->SetUniformMatrix4("u_viewProjectionMat", viewProjectionMatrix);
			material->GetShader()->SetUniformFloat3("u_cameraPosition", camera->GetPosition());
			mesh->Bind();

			RenderingAPI::GetRenderFunctions()->DrawIndexed(mesh->GetIndexCount());
		}
	}
}