#include "pch.h"
#include "Rendering/RenderingAPI.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanRenderer.h"
#include "Rendering/Vulkan/VulkanShader.h"
#include "Rendering/Vulkan/VulkanMesh.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanMaterial.h"

namespace Firefly
{
	RenderingAPI::Type RenderingAPI::s_type = RenderingAPI::Type::Vulkan;

	std::shared_ptr<GraphicsContext> RenderingAPI::CreateContext()
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanContext>();
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Renderer> RenderingAPI::CreateRenderer(std::shared_ptr<GraphicsContext> context)
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanRenderer>(context);
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Shader> RenderingAPI::CreateShader(std::shared_ptr<GraphicsContext> context)
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanShader>(context);
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Mesh> RenderingAPI::CreateMesh(std::shared_ptr<GraphicsContext> context)
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanMesh>(context);
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Texture> RenderingAPI::CreateTexture(std::shared_ptr<GraphicsContext> context)
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanTexture>(context);
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
		}
	}

	std::shared_ptr<Material> RenderingAPI::CreateMaterial(std::shared_ptr<GraphicsContext> context)
	{
		switch (s_type)
		{
		case RenderingAPI::Type::OpenGL:
		case RenderingAPI::Type::Vulkan:
			return std::make_shared<VulkanMaterial>(context);
		case RenderingAPI::Type::DirectX12:
		case RenderingAPI::Type::Metal:
		case RenderingAPI::Type::None:
		default:
			return nullptr;
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