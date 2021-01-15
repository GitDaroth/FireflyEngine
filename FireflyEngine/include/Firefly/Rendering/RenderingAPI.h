#pragma once

#include "Rendering/GraphicsContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"
#include "Rendering/Material.h"

namespace Firefly
{
	class RenderingAPI
	{
	public:
		enum Type
		{
			None,
			OpenGL,
			Vulkan,
			DirectX12,
			Metal
		};  

		static std::shared_ptr<GraphicsContext> CreateContext();
		static std::shared_ptr<Renderer> CreateRenderer(std::shared_ptr<GraphicsContext> context);
		static std::shared_ptr<Shader> CreateShader(std::shared_ptr<GraphicsContext> context);
		static std::shared_ptr<Mesh> CreateMesh(std::shared_ptr<GraphicsContext> context);
		static std::shared_ptr<Texture> CreateTexture(std::shared_ptr<GraphicsContext> context);
		static std::shared_ptr<Material> CreateMaterial(std::shared_ptr<GraphicsContext> context);

		static void SetType(Type type);
		static Type GetType();

	private:
		static Type s_type;
	};
}