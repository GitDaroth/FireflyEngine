#include "Scene/Entity.h"

#include "Scene/Scene.h"

namespace Firefly
{
	Entity::Entity(const Entity& entity) :
		m_entityRegistry(entity.m_entityRegistry),
		m_id(entity.m_id)
	{
	}

	Entity::Entity(std::shared_ptr<entt::registry> entityRegistry, entt::entity id) :
		m_entityRegistry(entityRegistry),
		m_id(id)
	{
		if(m_id == entt::null)
			m_id = m_entityRegistry->create();
	}

	Entity::Entity(std::shared_ptr<Scene> scene) :
		m_entityRegistry(scene->m_entityRegistry),
		m_id(entt::null)
	{
		m_id = m_entityRegistry->create();
	}

	Entity::~Entity()
	{
	}

	void Entity::RemoveFromScene()
	{
		m_entityRegistry->destroy(m_id);
	}
}