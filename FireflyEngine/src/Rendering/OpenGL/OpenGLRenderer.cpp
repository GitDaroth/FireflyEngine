#include "pch.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/OpenGL/OpenGLMesh.h"
#include "Rendering/OpenGL/OpenGLMaterial.h"
#include "Rendering/OpenGL/OpenGLShader.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/MeshComponent.h"
#include "Scene/Components/MaterialComponent.h"

#include <stb_image.h>

namespace Firefly
{
	OpenGLRenderer::OpenGLRenderer()
	{
		m_openGLContext = std::dynamic_pointer_cast<OpenGLContext>(RenderingAPI::GetContext());
		m_windowWidth = m_openGLContext->GetWidth();
		m_windowHeight = m_openGLContext->GetHeight();
	}

	void OpenGLRenderer::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// SHADERS
		ShaderCode shaderCode{};
		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/hdrImageToCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/hdrImageToCubeMap.frag");
		std::shared_ptr<OpenGLShader> hdrImageToCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("HdrImageToCubeMap", shaderCode));

		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/irradianceCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/irradianceCubeMap.frag");
		std::shared_ptr<OpenGLShader> irradianceCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("IrradianceCubeMap", shaderCode));

		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/environmentCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/environmentCubeMap.frag");
		m_environmentCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("EnvironmentCubeMap", shaderCode));

		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/prefilterCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/prefilterCubeMap.frag");
		std::shared_ptr<OpenGLShader> prefilterCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("PrefilterCubeMap", shaderCode));

		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/brdfLUT.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/brdfLUT.frag");
		std::shared_ptr<OpenGLShader> brdfLUTShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("BrdfLUTShader", shaderCode));

		// QUAD MESH
		m_quadVAO = 0;
		unsigned int quadVBO;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &m_quadVAO);
		glBindVertexArray(m_quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		// CUBE MESH
		m_cubeVAO = 0;
		unsigned int cubeVBO = 0;
		float vertices[] = {
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
		glGenVertexArrays(1, &m_cubeVAO);
		glBindVertexArray(m_cubeVAO);

		glGenBuffers(1, &cubeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		// fill buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		// LOAD HDR IMAGE
		//std::string environmentMapPath = "assets/textures/environment/FactoryCatwalk.hdr";
		std::string environmentMapPath = "assets/textures/environment/HamarikyuBridge.hdr";
		//std::string environmentMapPath = "assets/textures/environment/MonValley.hdr";
		//std::string environmentMapPath = "assets/textures/environment/TopangaForest.hdr";
		//std::string environmentMapPath = "assets/textures/environment/TropicalBeach.hdr";
		//std::string environmentMapPath = "assets/textures/environment/WinterForest.hdr";

		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* pixelData = stbi_loadf(environmentMapPath.c_str(), &width, &height, &nrComponents, 0);
		FIREFLY_ASSERT(pixelData, "Failed to load hdr image");

		uint32_t hdrTexture;
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, pixelData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(pixelData);

		// RENDER HDR IMAGE TO CUBE MAP
		unsigned int captureFBO, captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		glGenTextures(1, &m_envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		hdrImageToCubeMapShader->Bind();
		hdrImageToCubeMapShader->SetUniform("projection", captureProjection);
		hdrImageToCubeMapShader->SetUniform("hdrTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			hdrImageToCubeMapShader->SetUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render Cube
			glBindVertexArray(m_cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// COMPUTE IRRADIANCE CUBEMAP
		glGenTextures(1, &m_irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 64, 64, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 64, 64);

		irradianceCubeMapShader->Bind();
		irradianceCubeMapShader->SetUniform("environmentMap", 0);
		irradianceCubeMapShader->SetUniform("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

		glViewport(0, 0, 64, 64);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceCubeMapShader->SetUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render Cube
			glBindVertexArray(m_cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// COMPUTE PREFILTERED CUBEMAP
		glGenTextures(1, &m_prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		prefilterCubeMapShader->Bind();
		prefilterCubeMapShader->SetUniform("environmentMap", 0);
		prefilterCubeMapShader->SetUniform("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = 256 * std::pow(0.5, mip);
			unsigned int mipHeight = 256 * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterCubeMapShader->SetUniform("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				prefilterCubeMapShader->SetUniform("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				// render Cube
				glBindVertexArray(m_cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		// COMPUTE BRDF LUT
		glGenTextures(1, &m_brdfLUTTexture);

		glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1024, 1024, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUTTexture, 0);

		glViewport(0, 0, 1024, 1024);
		brdfLUTShader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render quad
		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_windowWidth, m_windowHeight);
	}

	void OpenGLRenderer::Destroy()
	{
	}

	void OpenGLRenderer::BeginDrawRecording()
	{
		m_entities.clear();
	}

	void OpenGLRenderer::RecordDraw(const Entity& entity)
	{
		if (entity.HasComponents<MeshComponent, MaterialComponent, TransformComponent>())
			m_entities.push_back(entity);
	}

	void OpenGLRenderer::EndDrawRecording()
	{
	}

	void OpenGLRenderer::SubmitDraw(std::shared_ptr<Camera> camera)
	{
		uint32_t newWindowWidth = m_openGLContext->GetWidth();
		uint32_t newWindowHeight = m_openGLContext->GetHeight();
		if (newWindowWidth == 0 || newWindowHeight == 0)
			return;

		if (newWindowWidth != m_windowWidth || newWindowHeight != m_windowHeight)
		{
			glViewport(0, 0, newWindowWidth, newWindowHeight);
			m_windowWidth = newWindowWidth;
			m_windowHeight = newWindowHeight;
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (size_t i = 0; i < m_entities.size(); i++)
		{
			glm::mat4 modelMatrix = m_entities[i].GetComponent<TransformComponent>().m_transform;
			std::shared_ptr<OpenGLMesh> mesh = std::dynamic_pointer_cast<OpenGLMesh>(m_entities[i].GetComponent<MeshComponent>().m_mesh);
			std::shared_ptr<OpenGLMaterial> material = std::dynamic_pointer_cast<OpenGLMaterial>(m_entities[i].GetComponent<MaterialComponent>().m_material);
			std::shared_ptr<OpenGLShader> shader = std::dynamic_pointer_cast<OpenGLShader>(material->GetShader());

			material->Bind();
			
			shader->SetUniform("irradianceMap", 6);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);

			shader->SetUniform("prefilterMap", 7);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);

			shader->SetUniform("brdfLUT", 8);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);

			shader->SetUniform("scene.viewMatrix", camera->GetViewMatrix());
			shader->SetUniform("scene.projectionMatrix", camera->GetProjectionMatrix());
			shader->SetUniform("scene.viewProjectionMatrix", camera->GetProjectionMatrix() * camera->GetViewMatrix());
			shader->SetUniform("scene.cameraPosition", glm::vec4(camera->GetPosition(), 1.0f));
			
			shader->SetUniform("object.modelMatrix", modelMatrix);
			shader->SetUniform("object.normalMatrix", glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMatrix)))));

			mesh->Bind();

			glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		}

		// RENDER ENVIRONMENT MAP AS BACKGROUND
		m_environmentCubeMapShader->Bind();
		m_environmentCubeMapShader->SetUniform("view", camera->GetViewMatrix());
		m_environmentCubeMapShader->SetUniform("projection", camera->GetProjectionMatrix());
		m_environmentCubeMapShader->SetUniform("environmentMap", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

		glBindVertexArray(m_cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		m_openGLContext->SwapBuffers();
	}
}