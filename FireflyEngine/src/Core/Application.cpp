#include "pch.h"
#include "Core/Application.h"

#include "Window/WindowsWindow.h"
#include "Event/WindowEvent.h"
#include "Input/Input.h"
#include "Rendering/RenderingAPI.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	Application::Application() : 
		m_lastFrameTime(0.f)
	{
		#ifdef FIREFLY_WINDOWS
			m_window = std::make_unique<WindowsWindow>();
		#endif

		m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		m_camera = std::make_shared<Camera>(m_window->GetWidth(), m_window->GetHeight());
		m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));

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

		std::string vertexShaderSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 texCoord;

			uniform mat4 viewProjectionMat;
			uniform mat4 modelMat;

			out vec2 v_texCoord;

			void main()
			{
				v_texCoord = texCoord;
				gl_Position = viewProjectionMat * modelMat * vec4(position, 1.0);
			}
		)";

		std::string fragmentShaderSource = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec2 v_texCoord;

			uniform sampler2D textureSampler;

			void main()
			{
				color = texture(textureSampler, v_texCoord);
			}
		)";

		m_shader = RenderingAPI::CreateShader();
		m_shader->Init(vertexShaderSource, fragmentShaderSource);

		m_texture = RenderingAPI::CreateTexture2D();
		m_texture->Init("assets/textures/test.jpg");

		m_shader->Bind();
		m_shader->SetUniformInt("textureSampler", 0);
	}

	Application::~Application()
	{
		for (auto layer : m_layers)
			layer->OnDetach();
	}

	void Application::Run()
	{
		m_lastFrameTime = (float)glfwGetTime();
		while (m_isRunning)
		{
			float time = (float)glfwGetTime();
			float deltaTime = time - m_lastFrameTime;
			m_lastFrameTime = time;

			//Logger::Debug("FireflyEngine", "{0} FPS ({1} ms)", 1.f / deltaTime, deltaTime * 1000.f);

			RenderingAPI::GetRenderFunctions()->SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
			RenderingAPI::GetRenderFunctions()->Clear();

			m_renderer->BeginScene(m_camera);
			m_texture->Bind(0);
			m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(1.1f, 0.f, 0.f)));
			m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f)));
			m_renderer->SubmitDraw(m_shader, m_vertexArray, glm::translate(glm::mat4(1.f), glm::vec3(-1.1f, 0.f, 0.f)));
			m_renderer->EndScene();

			UpdateLayers(deltaTime);
			m_window->OnUpdate();
		}
	}

	void Application::OnEvent(std::shared_ptr<Event> event)
	{
		Input::OnEvent(event);

		for (auto iter = m_layers.rbegin(); iter != m_layers.rend(); ++iter)
		{
			auto layer = (*iter);
			if (layer->IsEnabled())
			{
				layer->OnEvent(event);
				if (event->HasBeenHandled())
					break;
			}
		}

		if (event->IsType<WindowCloseEvent>())
			m_isRunning = false;
	}

	void Application::AddLayer(std::shared_ptr<Layer> layer)
	{
		m_layers.push_back(layer);
		layer->OnAttach();
	}

	void Application::RemoveLayer(std::shared_ptr<Layer> layer)
	{
		for (auto iter = m_layers.begin(); iter != m_layers.end(); ++iter)
		{
			if ((*iter) == layer)
			{
				(*iter)->OnDetach();
				m_layers.erase(iter);
				break;
			}
		}
	}

	void Application::RemoveLayer(const std::string& layerName)
	{
		for (auto iter = m_layers.begin(); iter != m_layers.end(); ++iter)
		{
			if ((*iter)->GetName() == layerName)
			{
				(*iter)->OnDetach();
				m_layers.erase(iter);
				break;
			}
		}
	}

	void Application::UpdateLayers(float deltaTime)
	{
		SortLayers();
		for (auto layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->OnUpdate(deltaTime);
		}
	}

	void Application::SortLayers()
	{
		std::sort(m_layers.begin(), m_layers.end(), [](const std::shared_ptr<Layer>& layer1, const std::shared_ptr<Layer>& layer2)
		{
			return layer1->GetOrderNumber() < layer2->GetOrderNumber();
		});
	}
}