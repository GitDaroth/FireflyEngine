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
#endif

#ifdef GFX_API_VULKAN
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanRenderer.h"
#include "Rendering/Vulkan/VulkanShader.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanMaterial.h"
#endif

namespace Firefly
{
#ifdef GFX_API_OPENGL
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::OpenGL;
#endif
#ifdef GFX_API_VULKAN
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::Vulkan;
#endif

	std::shared_ptr<GraphicsContext> RenderingAPI::CreateContext()
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLContext>();
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanContext>();
#endif
	}

	std::shared_ptr<Renderer> RenderingAPI::CreateRenderer(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLRenderer>(context);
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanRenderer>(context);
#endif
	}

	std::shared_ptr<Shader> RenderingAPI::CreateShader(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLShader>(context);
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanShader>(context);
#endif
	}

	std::shared_ptr<Mesh> RenderingAPI::CreateMesh(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLMesh>(context);
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanMesh>(context);
#endif
	}

	std::shared_ptr<Texture> RenderingAPI::CreateTexture(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLTexture>(context);
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanTexture>(context);
#endif
	}

	std::shared_ptr<Material> RenderingAPI::CreateMaterial(std::shared_ptr<GraphicsContext> context)
	{
#ifdef GFX_API_OPENGL
		return std::make_shared<OpenGLMaterial>(context);
#endif
#ifdef GFX_API_VULKAN
		return std::make_shared<VulkanMaterial>(context);
#endif
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