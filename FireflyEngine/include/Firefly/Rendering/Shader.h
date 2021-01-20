#pragma once

#include <string>
#include "Rendering/GraphicsContext.h"

namespace Firefly
{
	struct ShaderCode
	{
		std::vector<char> vertex;
		std::vector<char> tesselationControl;
		std::vector<char> tesselationEvaluation;
		std::vector<char> geometry;
		std::vector<char> fragment;
	};

	class Shader
	{
	public:
		void Init(const std::string& tag, const ShaderCode& shaderCode);
		virtual void Destroy() = 0;

		std::string GetTag() const;

		static std::vector<char> ReadShaderCodeFromFile(const std::string& path);

	protected:
		virtual void OnInit(const ShaderCode& shaderCode) = 0;

		std::string m_tag;
	};
}