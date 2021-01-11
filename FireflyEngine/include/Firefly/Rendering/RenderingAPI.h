#pragma once

#include "Rendering/GraphicsContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexArray.h"
#include "Rendering/VertexBuffer.h"
#include "Rendering/IndexBuffer.h"
#include "Rendering/RenderFunctions.h"
#include "Rendering/Texture.h"

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
		static std::shared_ptr<Renderer> CreateRenderer();
		static std::shared_ptr<Shader> CreateShader();
		static std::shared_ptr<Texture2D> CreateTexture2D();
		static std::shared_ptr<VertexArray> CreateVertexArray();
		static std::shared_ptr<VertexBuffer> CreateVertexBuffers();
		static std::shared_ptr<IndexBuffer> CreateIndexBuffers();
		static std::shared_ptr<RenderFunctions> GetRenderFunctions();

		static void SetType(Type type);
		static Type GetType();

	private:
		static Type s_type;
	};
}