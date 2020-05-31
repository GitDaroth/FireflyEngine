#include "pch.h"
#include "Application.h"

#include "Window/WindowsWindow.h"
#include "Event/WindowEvent.h"
#include "Input/Input.h"

namespace Firefly
{
	Application::Application()
	{
		#ifdef FIREFLY_WINDOWS
			m_window = std::make_unique<WindowsWindow>();
		#endif

		m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		m_renderer = std::make_unique<Renderer>();
		m_renderer->Init();
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
			m_renderer->Draw();

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