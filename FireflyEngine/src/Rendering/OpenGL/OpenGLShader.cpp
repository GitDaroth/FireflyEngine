#include "pch.h"
#include "Rendering/OpenGL/OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Firefly
{
	OpenGLShader::OpenGLShader()
	{
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_program);
		glDeleteShader(m_vertexShader);
		glDeleteShader(m_fragmentShader);
	}

	void OpenGLShader::Init(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
	{
		CompileShaders(vertexShaderSource, fragmentShaderSource);
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

	void OpenGLShader::SetUniformMatrix4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::CompileShaders(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
	{
		m_vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const GLchar* vertexSource = vertexShaderSource.c_str();
		glShaderSource(m_vertexShader, 1, &vertexSource, 0);

		glCompileShader(m_vertexShader);

		GLint hasBeenCompiled = 0;
		glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &hasBeenCompiled);
		if (hasBeenCompiled = GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(logLength);
			glGetShaderInfoLog(m_vertexShader, logLength, &logLength, &infoLog[0]);

			Logger::Error("FireflyEngine", "Failed to compile vertex shader:");
			Logger::Error("FireflyEngine", "  -> {0}", infoLog.data());

			glDeleteShader(m_vertexShader);

			return;
		}


		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar* fragmentSource = fragmentShaderSource.c_str();
		glShaderSource(m_fragmentShader, 1, &fragmentSource, 0);

		glCompileShader(m_fragmentShader);

		hasBeenCompiled = 0;
		glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &hasBeenCompiled);
		if (hasBeenCompiled = GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<GLchar> infoLog(logLength);
			glGetShaderInfoLog(m_fragmentShader, logLength, &logLength, &infoLog[0]);

			Logger::Error("FireflyEngine", "Failed to compile fragment shader:");
			Logger::Error("FireflyEngine", "  -> {0}", infoLog.data());

			glDeleteShader(m_vertexShader);
			glDeleteShader(m_fragmentShader);

			return;
		}
	}

	void OpenGLShader::LinkShaders()
	{
		m_program = glCreateProgram();

		glAttachShader(m_program, m_vertexShader);
		glAttachShader(m_program, m_fragmentShader);

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
			glDeleteShader(m_vertexShader);
			glDeleteShader(m_fragmentShader);

			return;
		}

		glDetachShader(m_program, m_vertexShader);
		glDetachShader(m_program, m_fragmentShader);
	}
}