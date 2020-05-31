#include "pch.h"
#include "Rendering/Renderer.h"

#include <glad/glad.h>

namespace Firefly
{
	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Init()
	{
		glGenVertexArrays(1, &m_vertexArray);
		glBindVertexArray(m_vertexArray);

		m_vertexBuffer = RenderingAPI::CreateVertexBuffer();
		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.f,
			 0.5f, -0.5f, 0.f,
			 0.f,   0.4f, 0.f
		};
		m_vertexBuffer->Init(vertices, sizeof(vertices));

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		m_indexBuffer = RenderingAPI::CreateIndexBuffer();
		unsigned int indices[3] = { 0, 1, 2 };
		m_indexBuffer->Init(indices, sizeof(indices));

		std::string vertexShaderSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 position;

			out vec3 pos;

			void main()
			{
				pos = position;
				gl_Position = vec4(position, 1.0);
			}
		)";

		std::string fragmentShaderSource = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 pos;

			void main()
			{
				color = vec4(pos * 0.5 + 0.5, 1.0);
			}
		)";

		m_shader = RenderingAPI::CreateShader();
		m_shader->Init(vertexShaderSource, fragmentShaderSource);
	}

	void Renderer::Draw()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		m_shader->Bind();
		glBindVertexArray(m_vertexArray);
		glDrawElements(GL_TRIANGLES, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}