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
		Component& GetComponent() const
		{
			auto view = m_entityRegistry->view<Component>();
			return view.get<Component>(m_id);
		}

		template<typename... Components>
		std::tuple<Components&...> GetComponents() const
		{
			auto view = m_entityRegistry->view<Components...>();
			return view.get<Components...>(m_id);
		}

		template<typename... Components>
		bool HasComponents() const
		{
			return m_entityRegistry->has<Components...>(m_id);
		}

	private:
		std::shared_ptr<entt::registry> m_entityRegistry;
		entt::entity m_id;
	};
}