#pragma once

#include "Core.h"
#include "pch.h"

class FIREFLY_API Event
{
public:
	template<typename T>
	bool IsType()
	{
		if (dynamic_cast<T*>(this) != nullptr)
			return true;
		else
			return false;
	}

	template<typename T>
	T* AsType()
	{
		return dynamic_cast<T*>(this);
	}

	virtual std::string ToString() const = 0;

protected:
	bool m_hasBeenHandled = false;
};

class FIREFLY_API InputEvent : public Event {};