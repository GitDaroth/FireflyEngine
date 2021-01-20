#pragma once

#include "Rendering/Mesh.h"

namespace Firefly
{
	class MeshGenerator
	{
	public:
		static std::shared_ptr<Mesh> CreateQuad(const glm::vec2& size = glm::vec3(1.0f), uint32_t subdivisions = 1);
		static std::shared_ptr<Mesh> CreateBox(const glm::vec3& size = glm::vec3(1.0f), uint32_t subdivisions = 1);
		static std::shared_ptr<Mesh> CreateSphere(float size = 1.0f, uint32_t latitude = 40, uint32_t longitude = 20);

	private:
		static float DegreeToRad(float angle);
	};
}