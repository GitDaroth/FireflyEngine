#pragma once

#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
    struct TransformComponent
    {
        glm::mat4 m_transform;

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;
        TransformComponent(const glm::mat4& transform) :
            m_transform(transform) {}
    };
}