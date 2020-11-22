#pragma once

#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	struct MaterialComponent
	{
		std::shared_ptr<Material> m_material;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent& other) = default;
		MaterialComponent(std::shared_ptr<Material> material) :
			m_material(material) {}
	};
}