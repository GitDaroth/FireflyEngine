#include "pch.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"

#include "Rendering/OpenGL/OpenGLMesh.h"
#include "Rendering/OpenGL/OpenGLMaterial.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/MaterialComponent.h"

namespace Firefly
{
	OpenGLRenderer::OpenGLRenderer(std::shared_ptr<GraphicsContext> context) :
		Renderer(context)
	{
		m_openGLContext = std::dynamic_pointer_cast<OpenGLContext>(context);
		m_windowWidth = m_openGLContext->GetWidth();
		m_windowHeight = m_openGLContext->GetHeight();
	}

	void OpenGLRenderer::Init()
	{
	}

	void OpenGLRenderer::Destroy()
	{
	}

	void OpenGLRenderer::BeginDrawRecording()
	{
		m_entities.clear();
	}

	void OpenGLRenderer::RecordDraw(const Entity& entity)
	{
		if (entity.HasComponents<MeshComponent, MaterialComponent, TransformComponent>())
			m_entities.push_back(entity);
	}

	void OpenGLRenderer::EndDrawRecording()
	{
	}

	void OpenGLRenderer::SubmitDraw(std::shared_ptr<Camera> camera)
	{
		uint32_t newWindowWidth = m_openGLContext->GetWidth();
		uint32_t newWindowHeight = m_openGLContext->GetHeight();
		if (newWindowWidth == 0 || newWindowHeight == 0)
			return;

		if (newWindowWidth != m_windowWidth || newWindowHeight != m_windowHeight)
		{
			glViewport(0, 0, newWindowWidth, newWindowHeight);
			m_windowWidth = newWindowWidth;
			m_windowHeight = newWindowHeight;
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (size_t i = 0; i < m_entities.size(); i++)
		{
			glm::mat4 modelMatrix = m_entities[i].GetComponent<TransformComponent>().m_transform;
			std::shared_ptr<OpenGLMesh> mesh = std::dynamic_pointer_cast<OpenGLMesh>(m_entities[i].GetComponent<MeshComponent>().m_mesh);
			std::shared_ptr<OpenGLMaterial> material = std::dynamic_pointer_cast<OpenGLMaterial>(m_entities[i].GetComponent<MaterialComponent>().m_material);
			std::shared_ptr<OpenGLShader> shader = std::dynamic_pointer_cast<OpenGLShader>(material->GetShader());

			material->Bind();
			
			shader->SetUniform("scene.viewMatrix", camera->GetViewMatrix());
			shader->SetUniform("scene.projectionMatrix", camera->GetProjectionMatrix());
			shader->SetUniform("scene.viewProjectionMatrix", camera->GetProjectionMatrix() * camera->GetViewMatrix());
			shader->SetUniform("scene.cameraPosition", glm::vec4(camera->GetPosition(), 1.0f));
			
			shader->SetUniform("object.modelMatrix", modelMatrix);
			shader->SetUniform("object.normalMatrix", glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMatrix)))));

			mesh->Bind();

			glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		}

		m_openGLContext->SwapBuffers();
	}
}