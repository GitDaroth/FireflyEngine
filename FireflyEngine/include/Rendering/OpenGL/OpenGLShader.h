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
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetUniformMatrix4(const std::string& name, const glm::mat4& matrix) override;

	private:
		void CompileShaders(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
		void LinkShaders();

		uint32_t m_vertexShader;
		uint32_t m_fragmentShader;
		uint32_t m_program;
	};
}