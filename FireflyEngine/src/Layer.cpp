#include "pch.h"
#include "Layer.h"

namespace Firefly
{
	Layer::Layer(const std::string& name, int orderNumber) :
		m_name(name),
		m_orderNumber(orderNumber),
		m_isEnabled(true)
	{
	}

	Layer::~Layer()
	{
	}

	void Layer::SetName(const std::string& name)
	{
		m_name = name;
	}

	void Layer::SetOrderNumber(int orderNumber)
	{
		m_orderNumber = orderNumber;
	}

	void Layer::Enable(bool isEnabled)
	{
		m_isEnabled = isEnabled;
	}

	std::string Layer::GetName() const
	{
		return m_name;
	}

	int Layer::GetOrderNumber() const
	{
		return m_orderNumber;
	}

	bool Layer::IsEnabled() const
	{
		return m_isEnabled;
	}
}