#include "pch.h"
#include "Rendering/RenderingAPI.h"

//#define GFX_API_OPENGL
#define GFX_API_VULKAN

#ifdef GFX_API_OPENGL
#include "Rendering/OpenGL/OpenGLContext.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Rendering/OpenGL/OpenGLMesh.h"
#include "Rendering/OpenGL/OpenGLTexture.h"
#include "Rendering/OpenGL/OpenGLMaterial.h"
#include "Rendering/OpenGL/OpenGLFrameBuffer.h"
#include "Rendering/OpenGL/OpenGLRenderPass.h"
#endif

#ifdef GFX_API_VULKAN
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanRenderer.h"
#include "Rendering/Vulkan/VulkanShader.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanMaterial.h"
//#include "Rendering/Vulkan/VulkanFrameBuffer.h"
//#include "Rendering/Vulkan/VulkanRenderPass.h"
#endif

namespace Firefly
{
	std::shared_ptr<GraphicsContext> RenderingAPI::s_context = nullptr;

#ifdef GFX_API_OPENGL
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::OpenGL;
#endif
#ifdef GFX_API_VULKAN
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::Vulkan;
#endif

	void RenderingAPI::Init(std::shared_ptr<Window> window)
	{
		s_context = CreateContext(window);
	}

	void RenderingAPI::Destroy()
	{
		s_context->Destroy();
	}

	std::shared_ptr<GraphicsContext> RenderingAPI::CreateContext(std::shared_ptr<Window> window)
	{
		std::shared_ptr<GraphicsContext> context;

		#ifdef GFX_API_OPENGL
			context = std::make_shared<OpenGLContext>();
		#endif
		#ifdef GFX_API_VULKAN
			context = std::make_shared<VulkanContext>();
		#endif

		context->Init(window);
		return context;
	}

	std::shared_ptr<Renderer> RenderingAPI::CreateRenderer()
	{
		std::shared_ptr<Renderer> renderer;

		#ifdef GFX_API_OPENGL
			renderer = std::make_shared<OpenGLRenderer>();
		#endif
		#ifdef GFX_API_VULKAN
			renderer = std::make_shared<VulkanRenderer>();
		#endif

		renderer->Init();
		return renderer;
	}

	std::shared_ptr<Shader> RenderingAPI::CreateShader(const std::string& tag, const ShaderCode& shaderCode)
	{
		std::shared_ptr<Shader> shader;

		#ifdef GFX_API_OPENGL
			shader = std::make_shared<OpenGLShader>();
		#endif
		#ifdef GFX_API_VULKAN
			shader = std::make_shared<VulkanShader>();
		#endif

		shader->Init(tag, shaderCode);
		return shader;
	}

	std::shared_ptr<Mesh> RenderingAPI::CreateMesh(std::vector<Mesh::Vertex> vertices, std::vector<uint32_t> indices)
	{
		std::shared_ptr<Mesh> mesh;

		#ifdef GFX_API_OPENGL
			mesh = std::make_shared<OpenGLMesh>();
		#endif
		#ifdef GFX_API_VULKAN
			mesh = std::make_shared<VulkanMesh>();
		#endif

		mesh->Init(vertices, indices);
		return mesh;
	}

	std::shared_ptr<Mesh> RenderingAPI::CreateMesh(const std::string& path, bool flipTexCoords)
	{
		std::shared_ptr<Mesh> mesh;

		#ifdef GFX_API_OPENGL
			mesh = std::make_shared<OpenGLMesh>();
		#endif
		#ifdef GFX_API_VULKAN
			mesh = std::make_shared<VulkanMesh>();
		#endif

		mesh->Init(path, flipTexCoords);
		return mesh;
	}

	std::shared_ptr<Texture> RenderingAPI::CreateTexture(const std::string& path, bool useLinearColorSpace)
	{
		std::shared_ptr<Texture> texture;

		#ifdef GFX_API_OPENGL
			texture = std::make_shared<OpenGLTexture>();
		#endif
		#ifdef GFX_API_VULKAN
			texture = std::make_shared<VulkanTexture>();
		#endif

		texture->Init(path, useLinearColorSpace);
		return texture;
	}

	std::shared_ptr<Texture> RenderingAPI::CreateTexture(const Texture::Description& description)
	{
		std::shared_ptr<Texture> texture;

		#ifdef GFX_API_OPENGL
			texture = std::make_shared<OpenGLTexture>();
		#endif
		#ifdef GFX_API_VULKAN
			texture = std::make_shared<VulkanTexture>();
		#endif

		texture->Init(description);
		return texture;
	}

	std::shared_ptr<Material> RenderingAPI::CreateMaterial(std::shared_ptr<Shader> shader)
	{
		std::shared_ptr<Material> material;

		#ifdef GFX_API_OPENGL
			material = std::make_shared<OpenGLMaterial>();
		#endif
		#ifdef GFX_API_VULKAN
			material = std::make_shared<VulkanMaterial>();
		#endif

		material->Init(shader);
		return material;
	}

	std::shared_ptr<FrameBuffer> RenderingAPI::CreateFrameBuffer(const FrameBuffer::Description& description)
	{
		std::shared_ptr<FrameBuffer> frameBuffer;

		#ifdef GFX_API_OPENGL
			frameBuffer = std::make_shared<OpenGLFrameBuffer>();
		#endif
		//#ifdef GFX_API_VULKAN
		//	frameBuffer = std::make_shared<VulkanFrameBuffer>();
		//#endif

		frameBuffer->Init(description);
		return frameBuffer;
	}

	std::shared_ptr<RenderPass> RenderingAPI::CreateRenderPass(const RenderPass::Description& description)
	{
		std::shared_ptr<RenderPass> renderPass;

		#ifdef GFX_API_OPENGL
			renderPass = std::make_shared<OpenGLRenderPass>();
		#endif
		//#ifdef GFX_API_VULKAN
		//	renderPass = std::make_shared<VulkanRenderPass>();
		//#endif

		renderPass->Init(description);
		return renderPass;
	}

	std::shared_ptr<GraphicsContext> RenderingAPI::GetContext()
	{
		return s_context;
	}

	void RenderingAPI::SetType(RenderingAPI::Type type)
	{
		s_type = type;
	}

	RenderingAPI::Type RenderingAPI::GetType()
	{
		return s_type;
	}
}