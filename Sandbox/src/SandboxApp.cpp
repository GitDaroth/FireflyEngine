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

		float vertices[5 * 4] = {
			-0.5f, -0.5f, 0.f, 0.f, 0.f,
			 0.5f, -0.5f, 0.f, 1.f, 0.f,
			 0.5f,  0.5f, 0.f, 1.f, 1.f,
			-0.5f,  0.5f, 0.f, 0.f, 1.f
		};
		std::shared_ptr<Firefly::VertexBuffer> vertexBuffer = Firefly::RenderingAPI::CreateVertexBuffer();
		vertexBuffer->Init(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ Firefly::ShaderDataType::Float3, "position" },
			{ Firefly::ShaderDataType::Float2, "texCoord" }
			});

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Firefly::IndexBuffer> indexBuffer = Firefly::RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices, sizeof(indices));

		m_vertexArray = Firefly::RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);

		m_shader = Firefly::RenderingAPI::CreateShader();
		m_shader->Init("assets/shaders/test.glsl");

		m_texture = Firefly::RenderingAPI::CreateTexture2D();
		m_texture->Init("assets/textures/test.jpg");

		m_shader->Bind();
		m_shader->SetUniformInt("textureSampler", 0);
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
		m_texture->Bind(0);
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(1.1f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(-1.1f, 0.f, 0.f)));
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
	std::shared_ptr<Firefly::Shader> m_shader;
	std::shared_ptr<Firefly::Texture2D> m_texture;
	std::shared_ptr<Firefly::VertexArray> m_vertexArray;
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}