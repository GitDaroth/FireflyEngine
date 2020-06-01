#pragma once

#include <glm/glm.hpp>

namespace Firefly
{
	class Shader
	{
	public:
		Shader() {}
		virtual ~Shader() {}

		virtual void Init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetUniformFloat(const std::string& name, const float value) = 0;
		virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetUniformMatrix3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetUniformMatrix4(const std::string& name, const glm::mat4& matrix) = 0;
	};
}