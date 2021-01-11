#include "pch.h"
#include "Rendering/RenderingAPI.h"

//#include "Rendering/OpenGL/OpenGLContext.h"
//#include "Rendering/OpenGL/OpenGLShader.h"
//#include "Rendering/OpenGL/OpenGLTexture.h"
//#include "Rendering/OpenGL/OpenGLVertexArray.h"
//#include "Rendering/OpenGL/OpenGLVertexBuffer.h"
//#include "Rendering/OpenGL/OpenGLIndexBuffer.h"
//#include "Rendering/OpenGL/OpenGLRenderFunctions.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

namespace Firefly
{
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::Vulkan;

	std::shared_ptr<GraphicsContext> RenderingAPI::CreateContext()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLContext>();
			//break;
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanContext>();
			break;
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<Renderer> RenderingAPI::CreateRenderer()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanRenderer>();
			break;
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<Shader> RenderingAPI::CreateShader()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLShader>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<Texture2D> RenderingAPI::CreateTexture2D()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLTexture2D>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<VertexArray> RenderingAPI::CreateVertexArray()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLVertexArray>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<VertexBuffer> RenderingAPI::CreateVertexBuffers()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLVertexBuffer>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<IndexBuffer> RenderingAPI::CreateIndexBuffers()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLIndexBuffer>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
	}

	std::shared_ptr<RenderFunctions> RenderingAPI::GetRenderFunctions()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
			//return std::make_shared<OpenGLRenderFunctions>();
			//break;
		case RenderingAPI::Type::Vulkan:
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
			break;
		}
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