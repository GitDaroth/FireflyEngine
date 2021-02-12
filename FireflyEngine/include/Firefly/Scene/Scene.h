#pragma once

#include <entt.hpp>

#include "Scene/Entity.h"

namespace Firefly
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

        std::vector<Entity> GetEntities();

        template<typename... Components>
        std::vector<Entity> GetEntityGroup()
        {
            std::vector<Entity> entityGroup;
            auto view = m_entityRegistry->view<Components...>();
            for (auto entityId : view)
                entityGroup.push_back(Entity(m_entityRegistry, entityId));
            return entityGroup;
        }

    private:
        std::shared_ptr<entt::registry> m_entityRegistry;

        friend class Entity;
    };
}