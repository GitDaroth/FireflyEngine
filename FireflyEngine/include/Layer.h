#pragma once

#include "Core.h"
#include "Event/Event.h"

namespace Firefly
{
	class Layer
	{
	public:
		Layer(const std::string& name, int orderNumber = 0);
		virtual ~Layer();

		void SetName(const std::string& name);
		void SetOrderNumber(int orderNumber);
		void Enable(bool isEnabled);

		std::string GetName() const;
		int GetOrderNumber() const;
		bool IsEnabled() const;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnEvent(std::shared_ptr<Event> event) = 0;

	protected:
		std::string m_name;
		int m_orderNumber;
		bool m_isEnabled;
	};
}