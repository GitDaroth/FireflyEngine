#pragma once

#include "Rendering/Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Firefly
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader();

		virtual void Destroy() override;

		void SetUniform(const std::string& name, const int value);
		void SetUniform(const std::string& name, const float value);
		void SetUniform(const std::string& name, const glm::vec2& value);
		void SetUniform(const std::string& name, const glm::vec3& value);
		void SetUniform(const std::string& name, const glm::vec4& value);
		void SetUniform(const std::string& name, const glm::mat3& value);
		void SetUniform(const std::string& name, const glm::mat4& value);

		void Bind();

	protected:
		virtual void OnInit(const ShaderCode& shaderCode) override;

	private:
		uint32_t CreateShaderModule(const std::vector<char>& shaderCode, GLenum shaderType);
		void LinkShaders();
		GLint GetUniformLocation(const std::string& name);
		static std::string GetStringFromShaderType(GLenum shaderType);

		std::vector<uint32_t> m_shaderModules;
		uint32_t m_program;
	};
}