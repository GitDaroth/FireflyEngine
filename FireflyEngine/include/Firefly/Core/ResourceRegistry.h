#pragma once

#include <memory>
#include <unordered_map>
#include <type_traits>

#include "Rendering/Shader.h"
#include "Rendering/Material.h"
#include "Rendering/Texture.h"
#include "Rendering/Mesh.h"

namespace Firefly
{
	template <typename TResource>
	class ResourceRegistry
	{
	public:
		static ResourceRegistry<TResource>& Instance()
		{
			static ResourceRegistry<TResource> registry;
			return registry;
		}

		void Insert(const std::string& name, std::shared_ptr<TResource> resource)
		{
			if (m_resources.find(name) == m_resources.end())
				m_resources[name] = resource;
		}

		std::shared_ptr<TResource> Retrieve(const std::string& name)
		{
			if (m_resources.find(name) != m_resources.end())
				return m_resources[name];
			return nullptr;
		}

		std::vector<std::shared_ptr<TResource>> GetAll() const
		{
			std::vector<std::shared_ptr<TResource>> resources;
			for (auto resourceEntry : m_resources)
				resources.push_back(resourceEntry.second);
			return resources;
		}

		void DestroyResources()
		{
			for (auto resourceEntry : m_resources)
				resourceEntry.second->Destroy();
			m_resources.clear();
		}

	private:
		ResourceRegistry() {};

		std::unordered_map<std::string, std::shared_ptr<TResource>> m_resources;
	};

	typedef ResourceRegistry<Shader> ShaderRegistry;
	typedef ResourceRegistry<Material> MaterialRegistry;
	typedef ResourceRegistry<Texture> TextureRegistry;
	typedef ResourceRegistry<Mesh> MeshRegistry;
}