#include "pch.h"
#include "Core/ImGuiLayer.h"

namespace Firefly
{
	ImGuiLayer::ImGuiLayer(const std::string& name, int orderNumber) :
		Layer(name, orderNumber)
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{

	}

	void ImGuiLayer::OnDetach()
	{

	}

	void ImGuiLayer::OnUpdate(float deltaTime)
	{

	}

	void ImGuiLayer::OnEvent(std::shared_ptr<Event> event)
	{

	}
}