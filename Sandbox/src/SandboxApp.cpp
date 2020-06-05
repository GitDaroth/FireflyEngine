#include <FireflyEngine.h>

#include <glm/gtc/matrix_transform.hpp>

#include "CameraController.h"

using namespace Firefly;

class SandboxApp : public Application
{
public:
	SandboxApp()
	{
		m_camera = std::make_shared<Camera>(m_window->GetWidth(), m_window->GetHeight());
		m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));

		m_cameraController = std::make_shared<CameraController>(m_camera);

		m_renderer = std::make_unique<Renderer>();

		float vertices[5 * 4] = {
			-0.5f, -0.5f, 0.f, 0.f, 0.f,
			 0.5f, -0.5f, 0.f, 1.f, 0.f,
			 0.5f,  0.5f, 0.f, 1.f, 1.f,
			-0.5f,  0.5f, 0.f, 0.f, 1.f
		};
		std::shared_ptr<VertexBuffer> vertexBuffer = RenderingAPI::CreateVertexBuffer();
		vertexBuffer->Init(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "position" },
			{ ShaderDataType::Float2, "texCoord" }
			});

		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<IndexBuffer> indexBuffer = RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices, sizeof(indices));

		m_vertexArray = RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);

		m_shader = RenderingAPI::CreateShader();
		m_shader->Init("assets/shaders/test.glsl");

		m_texture = RenderingAPI::CreateTexture2D();
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

		RenderingAPI::GetRenderFunctions()->SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		RenderingAPI::GetRenderFunctions()->Clear();

		m_renderer->BeginScene(m_camera);
		m_texture->Bind(0);
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(1.1f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f)));
		m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(-1.1f, 0.f, 0.f)));
		m_renderer->EndScene();
	}

	virtual void OnWindowEvent(std::shared_ptr<WindowEvent> event) override
	{
		if (auto resizeEvent = event->AsType<WindowResizeEvent>())
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

	virtual void OnKeyEvent(std::shared_ptr<KeyEvent> event) override
	{

	}

	virtual void OnMouseEvent(std::shared_ptr<MouseEvent> event) override
	{
		m_cameraController->OnMouseEvent(event);
	}

	std::unique_ptr<Renderer> m_renderer;
	std::shared_ptr<Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;
	std::shared_ptr<Shader> m_shader;
	std::shared_ptr<Texture2D> m_texture;
	std::shared_ptr<VertexArray> m_vertexArray;
};

Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}