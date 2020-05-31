#pragma once

namespace Firefly
{
	class Shader
	{
	public:
		Shader() {};
		virtual ~Shader() {};

		virtual void Init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};
}