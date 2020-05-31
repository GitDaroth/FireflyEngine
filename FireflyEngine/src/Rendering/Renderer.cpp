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
		float vertices[3 * 7] = {
			// position		   color
			-0.5f, -0.5f, 0.f, 0.8f, 0.f,  0.f,  1.f,
			 0.5f, -0.5f, 0.f, 0.8f, 0.8f, 0.f,  1.f,
			 0.f,   0.4f, 0.f, 0.8f, 0.f,  0.8f, 1.f
		};
		std::shared_ptr<VertexBuffer> vertexBuffer = RenderingAPI::CreateVertexBuffer();
		vertexBuffer->Init(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "position" },
			{ ShaderDataType::Float4, "color" }
		});

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> indexBuffer = RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices, sizeof(indices));

		m_vertexArray = RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexShaderSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 color;

			out vec3 pos;
			out vec4 col;

			void main()
			{
				pos = position;
				col = color;
				gl_Position = vec4(position, 1.0);
			}
		)";

		std::string fragmentShaderSource = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 pos;
			in vec4 col;

			void main()
			{
				color = col;
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
		m_vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, m_vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}