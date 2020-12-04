#include "Scene/Scene.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Scene::Scene()
	{
		m_entityRegistry = std::make_shared<entt::registry>();
	}

	Scene::~Scene()
	{
		m_entityRegistry->clear();
	}

	std::vector<Entity> Scene::GetEntities()
	{
		std::vector<Entity> entities;
		m_entityRegistry->each([this, &entities](auto entityId)
		{
			entities.push_back(Entity(m_entityRegistry, entityId));
		});
		return entities;
	}
}