#include "pch.h"
#include "Rendering/OpenGL/OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>

namespace Firefly
{
	OpenGLShader::OpenGLShader() :
		m_program(0)
	{
	}

	void OpenGLShader::OnInit(const ShaderCode& shaderCode)
	{
		if (!shaderCode.vertex.empty())
			m_shaderModules.push_back(CreateShaderModule(shaderCode.vertex, GL_VERTEX_SHADER));
		if (!shaderCode.tesselationControl.empty())
			m_shaderModules.push_back(CreateShaderModule(shaderCode.tesselationControl, GL_TESS_CONTROL_SHADER));
		if (!shaderCode.tesselationEvaluation.empty())
			m_shaderModules.push_back(CreateShaderModule(shaderCode.tesselationEvaluation, GL_TESS_EVALUATION_SHADER));
		if (!shaderCode.geometry.empty())
			m_shaderModules.push_back(CreateShaderModule(shaderCode.geometry, GL_GEOMETRY_SHADER));
		if (!shaderCode.fragment.empty())
			m_shaderModules.push_back(CreateShaderModule(shaderCode.fragment, GL_FRAGMENT_SHADER));

		LinkShaders();
	}

	void OpenGLShader::Destroy()
	{
		glDeleteProgram(m_program);
		for (auto shader : m_shaderModules)
			glDeleteShader(shader);
	}

	void OpenGLShader::SetUniform(const std::string& name, const int value)
	{
		glUniform1i(GetUniformLocation(name), value);
	}

	void OpenGLShader::SetUniform(const std::string& name, const float value)
	{
		glUniform1f(GetUniformLocation(name), value);
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec2& value)
	{
		glUniform2f(GetUniformLocation(name), value.x, value.y);
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec3& value)
	{
		glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::vec4& value)
	{
		glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::mat3& value)
	{
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetUniform(const std::string& name, const glm::mat4& value)
	{
		GLint test = GetUniformLocation(name);
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::Bind()
	{
		glUseProgram(m_program);
	}

	uint32_t OpenGLShader::CreateShaderModule(const std::vector<char>& shaderCode, GLenum shaderType)
	{
		GLuint shader = glCreateShader(shaderType);

		const GLchar* code = shaderCode.data();
		GLint codeSize = shaderCode.size();
		glShaderSource(shader, 1, &code, &codeSize);

		glCompileShader(shader);
		GLint hasBeenCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasBeenCompiled);
		if (hasBeenCompiled == GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(logLength);
			glGetShaderInfoLog(shader, logLength, &logLength, &infoLog[0]);

			glDeleteShader(shader);

			FIREFLY_ASSERT(hasBeenCompiled, "Unable to compile OpenGL {0} shader: {1}", GetStringFromShaderType(shaderType), infoLog.data());
		}
		
		return shader;
	}

	void OpenGLShader::LinkShaders()
	{
		m_program = glCreateProgram();

		for (auto shader : m_shaderModules)
			glAttachShader(m_program, shader);

		glLinkProgram(m_program);

		GLint hasBeenLinked = 0;
		glGetProgramiv(m_program, GL_LINK_STATUS, &hasBeenLinked);
		if (hasBeenLinked == GL_FALSE)
		{
			GLint logLength = 0;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(logLength);
			glGetProgramInfoLog(m_program, logLength, &logLength, &infoLog[0]);

			glDeleteProgram(m_program);
			for (auto shader : m_shaderModules)
				glDeleteShader(shader);

			FIREFLY_ASSERT(hasBeenLinked, "Unable to link OpenGL shader program: {0}", infoLog.data());
		}

		for (auto shader : m_shaderModules)
			glDetachShader(m_program, shader);
	}

	GLint OpenGLShader::GetUniformLocation(const std::string& name)
	{
		return glGetUniformLocation(m_program, name.c_str());
	}

	std::string OpenGLShader::GetStringFromShaderType(GLenum shaderType)
	{
		std::string shaderName = "unknown";

		switch (shaderType)
		{
		case GL_VERTEX_SHADER:
			shaderName = "vertex";
			break;
		case GL_TESS_CONTROL_SHADER:
			shaderName = "tesselation control";
			break;
		case GL_TESS_EVALUATION_SHADER:
			shaderName = "tesselation evaluation";
			break;
		case GL_GEOMETRY_SHADER:
			shaderName = "geometry";
			break;
		case GL_FRAGMENT_SHADER:
			shaderName = "fragment";
			break;
		}

		return shaderName;
	}
}