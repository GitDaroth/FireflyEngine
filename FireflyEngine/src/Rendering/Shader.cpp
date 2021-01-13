#include "pch.h"
#include "Rendering/Shader.h"

#include <fstream>

namespace Firefly
{
	Shader::Shader(std::shared_ptr<GraphicsContext> context) :
		m_context(context)
	{
	}

	void Shader::Init(const std::string& tag, const ShaderCode& shaderCode)
	{
		m_tag = tag;
		OnInit(shaderCode);
	}

	std::string Shader::GetTag() const
	{
		return m_tag;
	}

	std::vector<char> Shader::ReadShaderCodeFromFile(const std::string& path)
	{
		std::vector<char> shaderCode;

		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			Logger::Error("FireflyEngine", "Failed to open shader file: {0}!", path);
			return shaderCode;
		}

		size_t fileSize = (size_t)file.tellg();
		shaderCode.resize(fileSize);
		file.seekg(0);
		file.read(shaderCode.data(), fileSize);
		file.close();

		return shaderCode;
	}
}