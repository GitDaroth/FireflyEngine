#pragma once

#include <entt.hpp>

namespace Firefly
{
	class Scene
	{
	public:
		Scene();
		~Scene();

	public:
		entt::registry m_entityRegistry;
	};
}