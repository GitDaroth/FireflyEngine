#pragma once

#include "Window/Window.h"
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

		static void Init(std::shared_ptr<Window> window);
		static void Destroy();

		static std::shared_ptr<Renderer> CreateRenderer();
		static std::shared_ptr<Shader> CreateShader(const std::string& tag, const ShaderCode& shaderCode);
		static std::shared_ptr<Mesh> CreateMesh(std::vector<Mesh::Vertex> vertices, std::vector<uint32_t> indices);
		static std::shared_ptr<Mesh> CreateMesh(const std::string& path, bool flipTexCoords = false);
		static std::shared_ptr<Texture> CreateTexture(const std::string& path, Texture::ColorSpace colorSpace = Texture::ColorSpace::RGB);
		static std::shared_ptr<Material> CreateMaterial(std::shared_ptr<Shader> shader);

		static std::shared_ptr<GraphicsContext> GetContext();

		static void SetType(Type type);
		static Type GetType();

	private:
		static std::shared_ptr<GraphicsContext> CreateContext(std::shared_ptr<Window> window);

		static Type s_type;
		static std::shared_ptr<GraphicsContext> s_context;
	};
}