#pragma once

#include "Layer.h"

namespace Firefly
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const std::string& name, int orderNumber = 0);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnEvent(std::shared_ptr<Event> event) override;
	};
}