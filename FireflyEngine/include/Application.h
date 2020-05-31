#pragma once

#include "Core.h"
#include "Window/Window.h"
#include "Layer.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"

namespace Firefly
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(std::shared_ptr<Event> event);

		void AddLayer(std::shared_ptr<Layer> layer);
		void RemoveLayer(std::shared_ptr<Layer> layer);
		void RemoveLayer(const std::string& layerName);

	private:
		void SortLayers();

		std::unique_ptr<Window> m_window;
		bool m_isRunning = true;
		std::vector<std::shared_ptr<Layer>> m_layers;	// front is background, back is foreground

		std::unique_ptr<Renderer> m_renderer;
		std::shared_ptr<Camera> m_camera;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<VertexArray> m_vertexArray;
	};
}