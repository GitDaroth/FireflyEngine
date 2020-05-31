#include "pch.h"
#include "Application.h"

#include "Window/WindowsWindow.h"
#include "Event/WindowEvent.h"
#include "Input/Input.h"
#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Application::Application()
	{
		#ifdef FIREFLY_WINDOWS
			m_window = std::make_unique<WindowsWindow>();
		#endif

		m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		m_renderer = std::make_unique<Renderer>();
		
		float vertices[3 * 7] = {
			// position		   color
			-0.5f, -0.5f, 0.f, 0.8f, 0.f,  0.f,  1.f,
			 0.5f, -0.5f, 0.f, 0.8f, 0.8f, 0.f,  1.f,
			 0.f,   0.4f, 0.f, 0.8f, 0.f,  0.8f, 1.f
		};
		std::shared_ptr<VertexBuffer> vertexBuffer = RenderingAPI::CreateVertexBuffer();
		vertexBuffer->Init(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "position" },
			{ ShaderDataType::Float4, "color" }
		});

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> indexBuffer = RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices, sizeof(indices));

		m_vertexArray = RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexShaderSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 color;

			out vec3 pos;
			out vec4 col;

			void main()
			{
				pos = position;
				col = color;
				gl_Position = vec4(position, 1.0);
			}
		)";

		std::string fragmentShaderSource = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 pos;
			in vec4 col;

			void main()
			{
				color = col;
			}
		)";

		m_shader = RenderingAPI::CreateShader();
		m_shader->Init(vertexShaderSource, fragmentShaderSource);
	}

	Application::~Application()
	{
		for (auto layer : m_layers)
			layer->OnDetach();
	}

	void Application::Run()
	{
		while (m_isRunning)
		{
			RenderingAPI::GetRenderFunctions()->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });
			RenderingAPI::GetRenderFunctions()->Clear();

			m_renderer->BeginScene();
			m_shader->Bind();
			m_renderer->SubmitDraw(m_vertexArray);
			m_renderer->EndScene();

			SortLayers();
			for (auto layer : m_layers)
			{
				if(layer->IsEnabled())
					layer->OnUpdate();
			}
				
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

	void Application::SortLayers()
	{
		std::sort(m_layers.begin(), m_layers.end(), [](const std::shared_ptr<Layer>& layer1, const std::shared_ptr<Layer>& layer2)
		{
			return layer1->GetOrderNumber() < layer2->GetOrderNumber();
		});
	}
}