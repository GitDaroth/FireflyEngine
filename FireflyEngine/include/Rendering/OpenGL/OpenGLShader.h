#pragma once

#include "Rendering/Shader.h"

namespace Firefly
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader();
		virtual ~OpenGLShader() override;

		virtual void Init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) override;
		virtual void Init(const std::string& path) override;
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetUniformInt(const std::string& name, const int value) override;
		virtual void SetUniformFloat(const std::string& name, const float value) override;
		virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetUniformMatrix3(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMatrix4(const std::string& name, const glm::mat4& matrix) override;

	private:
		void CompileShaders(const std::unordered_map<uint32_t, std::string>& shaderSources);
		void LinkShaders();

		std::vector<uint32_t> m_shaders;
		uint32_t m_program;
	};
}