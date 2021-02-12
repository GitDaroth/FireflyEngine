#pragma once

#include "pch.h"
#include "Rendering/Mesh.h"

namespace Firefly
{
    struct MeshComponent
    {
        std::shared_ptr<Mesh> m_mesh;

        MeshComponent() = default;
        MeshComponent(const MeshComponent& other) = default;
        MeshComponent(std::shared_ptr<Mesh> mesh) :
            m_mesh(mesh) {}
    };
}