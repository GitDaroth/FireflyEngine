#pragma once

#include "pch.h"

#include <entt.hpp>

namespace Firefly
{	
	class Scene;

	class Entity
	{
	public:
		Entity(const Entity& entity);
		Entity(std::shared_ptr<entt::registry> entityRegistry, entt::entity id = entt::null);
		Entity(std::shared_ptr<Scene> scene);
		~Entity();

		Entity Clone();
		void RemoveFromScene();

		template<typename Component, typename... Args>
		void AddComponent(Args... args)
		{
			m_entityRegistry->emplace<Component>(m_id, args...);
		}

		template<typename Component>
		void RemoveComponent()
		{
			m_entityRegistry->remove_if_exists<Component>();
		}

		template<typename Component>
		Component& GetComponent()
		{
			auto view = m_entityRegistry->view<Component>();
			return view.get<Component>(m_id);
		}

		template<typename... Components>
		std::tuple<Components&...> GetComponents()
		{
			auto view = m_entityRegistry->view<Components...>();
			return view.get<Components...>(m_id);
		}

	private:
		std::shared_ptr<entt::registry> m_entityRegistry;
		entt::entity m_id;
	};
}