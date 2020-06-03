#include "pch.h"
#include "Rendering/OpenGL/OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Firefly
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;

		Logger::Error("FireflyEngine", "Unknown shader type:{0}", type);
		return 0;
	}

	static const std::string& StringFromShaderType(GLenum shaderType)
	{
		if (shaderType == GL_VERTEX_SHADER)
			return "vertex";
		if (shaderType == GL_FRAGMENT_SHADER)
			return "fragment";

		Logger::Error("FireflyEngine", "Unknown shader type:{0}", shaderType);
		return 0;
	}

	OpenGLShader::OpenGLShader()
	{
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_program);
		for (auto shader : m_shaders)
			glDeleteShader(shader);
	}

	void OpenGLShader::Init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
	{
		std::unordered_map<uint32_t, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexShaderSource;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentShaderSource;

		CompileShaders(shaderSources);
		LinkShaders();
	}

	void OpenGLShader::Init(const std::string& path)
	{
		std::string shaderSource;

		std::ifstream in(path, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			shaderSource.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&shaderSource[0], shaderSource.size());
			in.close();
		}
		else
			Logger::Error("FireflyEngine", "Unable to open shader file: {0}", path);


		std::unordered_map<uint32_t, std::string> shaderSources;

		const char* typeKeyword = "#type";
		size_t typeKeywordLength = strlen(typeKeyword);
		size_t pos = shaderSource.find(typeKeyword, 0);
		while (pos != std::string::npos)
		{
			size_t eol = shaderSource.find_first_of("\r\n", pos);
			size_t begin = pos + typeKeywordLength + 1;
			std::string type = shaderSource.substr(begin, eol - begin);

			size_t nextLinePos = shaderSource.find_first_not_of("\r\n", eol);
			pos = shaderSource.find(typeKeyword, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = 
				shaderSource.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? shaderSource.size() - 1 : nextLinePos));
		}

		CompileShaders(shaderSources);
		LinkShaders();
	}

	void OpenGLShader::Bind()
	{
		glUseProgram(m_program);
	}

	void OpenGLShader::Unbind()
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetUniformInt(const std::string& name, const int value)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::SetUniformFloat(const std::string& name, const float value)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetUniformMatrix3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetUniformMatrix4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::CompileShaders(const std::unordered_map<uint32_t, std::string>& shaderSources)
	{
		for (auto& shaderSource : shaderSources)
		{
			GLenum type = shaderSource.first;
			const std::string& source = shaderSource.second;

			GLuint shader = glCreateShader(type);

			const GLchar* src = source.c_str();
			glShaderSource(shader, 1, &src, 0);

			glCompileShader(shader);

			GLint hasBeenCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &hasBeenCompiled);
			if (hasBeenCompiled = GL_FALSE)
			{
				GLint logLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

				std::vector<GLchar> infoLog(logLength);
				glGetShaderInfoLog(shader, logLength, &logLength, &infoLog[0]);

				Logger::Error("FireflyEngine", "Failed to compile {0} shader:", StringFromShaderType(type));
				Logger::Error("FireflyEngine", "  -> {0}", infoLog.data());

				glDeleteShader(shader);

				break;
			}

			m_shaders.push_back(shader);
		}
	}

	void OpenGLShader::LinkShaders()
	{
		m_program = glCreateProgram();

		for(auto shader : m_shaders)
			glAttachShader(m_program, shader);

		glLinkProgram(m_program);

		GLint hasBeenLinked = 0;
		glGetShaderiv(m_program, GL_LINK_STATUS, &hasBeenLinked);
		if (hasBeenLinked = GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(m_program, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(logLength);
			glGetShaderInfoLog(m_program, logLength, &logLength, &infoLog[0]);

			Logger::Error("FireflyEngine", "Failed to link shader program:");
			Logger::Error("FireflyEngine", "  -> {0}", infoLog.data());

			glDeleteProgram(m_program);
			for (auto shader : m_shaders)
				glDeleteShader(shader);

			return;
		}

		for (auto shader : m_shaders)
			glDetachShader(m_program, shader);
	}
}