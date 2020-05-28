#pragma once

#include "Core.h"
#include "pch.h"

class FIREFLY_API Event : public std::enable_shared_from_this<Event>
{
public:
	template<typename T>
	bool IsType()
	{
		if (std::dynamic_pointer_cast<T>(shared_from_this()) != nullptr)
			return true;
		else
			return false;
	}

	template<typename T>
	std::shared_ptr<T> AsType()
	{
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

	virtual std::string ToString() const = 0;

protected:
	bool m_hasBeenHandled = false;
};

class FIREFLY_API InputEvent : public Event {};