#include "pch.h"
#include "Rendering/Renderer.h"

#include "Rendering/RenderingAPI.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/MaterialComponent.h"
#include "Scene/Components/TransformComponent.h"

namespace Firefly
{
	//Renderer::Renderer()
	//{
	//	RenderingAPI::GetRenderFunctions()->Init();
	//}

	//Renderer::~Renderer()
	//{
	//}

	//void Renderer::DrawScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera)
	//{
	//	glm::mat4 viewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();

	//	auto entityGroup = scene->GetEntityGroup<MeshComponent, MaterialComponent, TransformComponent>();
	//	for (auto entity : entityGroup)
	//	{
	//		auto [meshComp, materialComp, transformComp] = entity.GetComponents<MeshComponent, MaterialComponent, TransformComponent>();
	//		auto mesh = meshComp.m_mesh;
	//		auto material = materialComp.m_material;
	//		auto transform = transformComp.m_transform;
	//		material->Bind();
	//		material->GetShader()->SetUniformMatrix4("u_modelMat", transform);
	//		material->GetShader()->SetUniformMatrix3("u_normalMat", glm::transpose(glm::inverse(glm::mat3(transform))));
	//		material->GetShader()->SetUniformMatrix4("u_viewProjectionMat", viewProjectionMatrix);
	//		material->GetShader()->SetUniformFloat3("u_cameraPosition", camera->GetPosition());
	//		mesh->Bind();

	//		RenderingAPI::GetRenderFunctions()->DrawIndexed(mesh->GetIndexCount());
	//	}
	//}
}