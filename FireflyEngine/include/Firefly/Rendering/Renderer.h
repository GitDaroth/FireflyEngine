#pragma once

#include "Scene/Scene.h"
#include "Rendering/GraphicsContext.h"
#include "Rendering/Shader.h"
#include "Scene/Camera.h"

namespace Firefly
{
	struct SceneData
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 viewProjectionMatrix;
		glm::vec4 cameraPosition;
	};

	struct MaterialData
	{
		glm::vec4 albedo;
		float roughness;
		float metalness;
		float heightScale;
		float hasAlbedoTexture;
		float hasNormalTexture;
		float hasRoughnessTexture;
		float hasMetalnessTexture;
		float hasOcclusionTexture;
		float hasHeightTexture;
	};

	struct ObjectData
	{
		glm::mat4 modelMatrix;
		glm::mat4 normalMatrix;
	};

	class Renderer
	{
	public:
		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual void BeginDrawRecording() = 0;
		virtual void RecordDraw(const Entity& entity) = 0;
		virtual void EndDrawRecording() = 0;
		virtual void SubmitDraw(std::shared_ptr<Camera> camera) = 0;
	};
}