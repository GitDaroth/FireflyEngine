#include <FireflyEngine.h>

#include "CameraController.h"

#include <glm/gtc/matrix_transform.hpp>

class SandboxApp : public Firefly::Application
{
public:
	SandboxApp()
	{
		m_camera = std::make_shared<Firefly::Camera>(m_window->GetWidth(), m_window->GetHeight());
		m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));

		m_cameraController = std::make_shared<CameraController>(m_camera);

		m_renderer = std::make_unique<Firefly::Renderer>();

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
		std::vector<Firefly::Mesh::Vertex> vertices = {
			// position, normal, texCoords
			{ {-0.5f, -0.5f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
			{ {0.5f, -0.5f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 0.f} },
			{ {0.5f,  0.5f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f} },
			{ {-0.5f,  0.5f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 1.f} }
		};

		std::shared_ptr<Firefly::Mesh> mesh = std::make_shared<Firefly::Mesh>(vertices, indices);

		std::shared_ptr<Firefly::Shader> shader = Firefly::RenderingAPI::CreateShader();
		shader->Init("assets/shaders/test.glsl");

		std::shared_ptr<Firefly::Texture2D> texture = Firefly::RenderingAPI::CreateTexture2D();
		texture->Init("assets/textures/test.jpg");

		std::shared_ptr<Firefly::Material> material = std::make_shared<Firefly::Material>(shader);
		material->SetDiffuseTexture(texture);

		m_model = std::make_shared<Firefly::Model>(mesh, material);
	}

	~SandboxApp()
	{
	}

protected:
	virtual void OnUpdate(float deltaTime) override
	{
		m_cameraController->OnUpdate(deltaTime);

		Firefly::RenderingAPI::GetRenderFunctions()->SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		Firefly::RenderingAPI::GetRenderFunctions()->Clear();

		m_renderer->BeginScene(m_camera);
		m_model->SetModelMatrix(glm::translate(glm::mat4(1), glm::vec3(-1.1f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_model);
		m_model->SetModelMatrix(glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_model);
		m_model->SetModelMatrix(glm::translate(glm::mat4(1), glm::vec3(1.1f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_model);
		m_renderer->EndScene();
	}

	virtual void OnWindowEvent(std::shared_ptr<Firefly::WindowEvent> event) override
	{
		if (auto resizeEvent = event->AsType<Firefly::WindowResizeEvent>())
		{
			int width = resizeEvent->GetWidth();
			int height = resizeEvent->GetHeight();

			if (width != 0 && height != 0)
			{
				m_camera->SetWidth(width);
				m_camera->SetHeight(height);
			}
		}
	}

	virtual void OnKeyEvent(std::shared_ptr<Firefly::KeyEvent> event) override
	{

	}

	virtual void OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event) override
	{
		m_cameraController->OnMouseEvent(event);
	}

	std::unique_ptr<Firefly::Renderer> m_renderer;
	std::shared_ptr<Firefly::Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;
	std::shared_ptr<Firefly::Model> m_model;
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}