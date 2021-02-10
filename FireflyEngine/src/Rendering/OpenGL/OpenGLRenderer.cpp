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
		CreateRenderPass();
		CreateFrameBuffer();
		CreatePBRShaderResources();

		ShaderCode shaderCode{};
		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/screenTexture.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/screenTexture.frag");
		m_screenTextureShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("ScreenTexture", shaderCode));
	}

	void OpenGLRenderer::Destroy()
	{
		m_screenTextureShader->Destroy();

		DestroyPBRShaderResources();
		DestroyFrameBuffer();
		DestroyRenderPass();
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
			m_windowWidth = newWindowWidth;
			m_windowHeight = newWindowHeight;

			DestroyFrameBuffer();
			CreateFrameBuffer();
		}

		m_mainRenderPass->Begin(m_mainFrameBuffer);

		for (size_t i = 0; i < m_entities.size(); i++)
		{
			glm::mat4 modelMatrix = m_entities[i].GetComponent<TransformComponent>().m_transform;
			std::shared_ptr<OpenGLMesh> mesh = std::dynamic_pointer_cast<OpenGLMesh>(m_entities[i].GetComponent<MeshComponent>().m_mesh);
			std::shared_ptr<OpenGLMaterial> material = std::dynamic_pointer_cast<OpenGLMaterial>(m_entities[i].GetComponent<MaterialComponent>().m_material);
			std::shared_ptr<OpenGLShader> shader = std::dynamic_pointer_cast<OpenGLShader>(material->GetShader());

			material->Bind();

			shader->SetUniform("irradianceMap", 6);
			m_irradianceCubeMap->Bind(6);

			shader->SetUniform("prefilterMap", 7);
			m_prefilterCubeMap->Bind(7);

			shader->SetUniform("brdfLUT", 8);
			m_brdfLUT->Bind(8);

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
		m_environmentCubeMap->Bind(0);

		glBindVertexArray(m_cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		m_mainRenderPass->End();

		// RENDER RESOLVED COLOR TEXTURE TO SCREEN
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_windowWidth, m_windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_screenTextureShader->Bind();
		m_screenTextureShader->SetUniform("screenTexture", 0);
		m_colorResolveTexture->Bind(0);

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		m_openGLContext->SwapBuffers();
	}

	void OpenGLRenderer::CreateRenderPass()
	{
		RenderPass::Description mainRenderPassDesc = {};
		mainRenderPassDesc.isDepthTestingEnabled = true;
		mainRenderPassDesc.depthCompareOperation = CompareOperation::LESS_OR_EQUAL;
		mainRenderPassDesc.isMultisamplingEnabled = true;
		mainRenderPassDesc.isSampleShadingEnabled = true;
		mainRenderPassDesc.minSampleShading = 1.0f;
		mainRenderPassDesc.colorAttachmentLayouts = { {Texture::Format::RGBA_8, m_msaaSampleCount} };
		mainRenderPassDesc.colorResolveAttachmentLayouts = { {Texture::Format::RGBA_8, Texture::SampleCount::SAMPLE_1} };
		mainRenderPassDesc.depthStencilAttachmentLayout = { Texture::Format::DEPTH_32_FLOAT, m_msaaSampleCount };
		m_mainRenderPass = RenderingAPI::CreateRenderPass(mainRenderPassDesc);

		ShaderCode shaderCode{};
		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/environmentCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/environmentCubeMap.frag");
		m_environmentCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("EnvironmentCubeMap", shaderCode));
	}

	void OpenGLRenderer::DestroyRenderPass()
	{
		m_environmentCubeMapShader->Destroy();
		m_mainRenderPass->Destroy();
	}

	void OpenGLRenderer::CreateFrameBuffer()
	{
		Texture::Description colorTextureDesc = {};
		colorTextureDesc.type = Texture::Type::TEXTURE_2D;
		colorTextureDesc.width = m_windowWidth;
		colorTextureDesc.height = m_windowHeight;
		colorTextureDesc.format = Texture::Format::RGBA_8;
		colorTextureDesc.sampleCount = m_msaaSampleCount;
		colorTextureDesc.useAsAttachment = true;
		colorTextureDesc.useSampler = false;
		m_colorTexture = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(colorTextureDesc));

		Texture::Description colorResolveTextureDesc = {};
		colorResolveTextureDesc.type = Texture::Type::TEXTURE_2D;
		colorResolveTextureDesc.width = m_windowWidth;
		colorResolveTextureDesc.height = m_windowHeight;
		colorResolveTextureDesc.format = Texture::Format::RGBA_8;
		colorResolveTextureDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		colorResolveTextureDesc.useAsAttachment = true;
		colorResolveTextureDesc.useSampler = true;
		colorResolveTextureDesc.sampler.isMipMappingEnabled = false;
		colorResolveTextureDesc.sampler.isAnisotropicFilteringEnabled = false;
		colorResolveTextureDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		colorResolveTextureDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		colorResolveTextureDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		m_colorResolveTexture = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(colorResolveTextureDesc));

		Texture::Description depthTextureDesc = {};
		depthTextureDesc.type = Texture::Type::TEXTURE_2D;
		depthTextureDesc.width = m_windowWidth;
		depthTextureDesc.height = m_windowHeight;
		depthTextureDesc.format = Texture::Format::DEPTH_32_FLOAT;
		depthTextureDesc.sampleCount = m_msaaSampleCount;
		depthTextureDesc.useAsAttachment = true;
		depthTextureDesc.useSampler = false;
		m_depthTexture = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(depthTextureDesc));

		FrameBuffer::Attachment colorAttachment;
		colorAttachment.texture = m_colorTexture;

		FrameBuffer::Attachment colorResolveAttachment;
		colorResolveAttachment.texture = m_colorResolveTexture;

		FrameBuffer::Attachment depthAttachment;
		depthAttachment.texture = m_depthTexture;

		FrameBuffer::Description frameBufferDesc = {};
		frameBufferDesc.width = m_windowWidth;
		frameBufferDesc.height = m_windowHeight;
		frameBufferDesc.colorAttachments = { colorAttachment };
		frameBufferDesc.colorResolveAttachments = { colorResolveAttachment };
		frameBufferDesc.depthStencilAttachment = { depthAttachment };
		m_mainFrameBuffer = RenderingAPI::CreateFrameBuffer(frameBufferDesc);
	}

	void OpenGLRenderer::DestroyFrameBuffer()
	{
		m_mainFrameBuffer->Destroy();
		m_depthTexture->Destroy();
		m_colorResolveTexture->Destroy();
		m_colorTexture->Destroy();
	}

	void OpenGLRenderer::CreatePBRShaderResources()
	{
		// SHADERS
		ShaderCode shaderCode{};
		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/hdrImageToCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/hdrImageToCubeMap.frag");
		std::shared_ptr<OpenGLShader> hdrImageToCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("HdrImageToCubeMap", shaderCode));

		shaderCode.vertex = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/irradianceCubeMap.vert");
		shaderCode.fragment = Shader::ReadShaderCodeFromFile("assets/shaders/OpenGL/irradianceCubeMap.frag");
		std::shared_ptr<OpenGLShader> irradianceCubeMapShader = std::dynamic_pointer_cast<OpenGLShader>(RenderingAPI::CreateShader("IrradianceCubeMap", shaderCode));

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
		//std::string environmentMapPath = "assets/textures/environment/HamarikyuBridge.hdr";
		//std::string environmentMapPath = "assets/textures/environment/MonValley.hdr";
		std::string environmentMapPath = "assets/textures/environment/TopangaForest.hdr";
		//std::string environmentMapPath = "assets/textures/environment/TropicalBeach.hdr";
		//std::string environmentMapPath = "assets/textures/environment/WinterForest.hdr";

		std::shared_ptr<OpenGLTexture> hdrTexture = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(environmentMapPath));

		RenderPass::Description imageBasedLightingRenderPassDesc = {};
		imageBasedLightingRenderPassDesc.isDepthTestingEnabled = false;
		imageBasedLightingRenderPassDesc.isMultisamplingEnabled = false;
		imageBasedLightingRenderPassDesc.colorAttachmentLayouts = { {Texture::Format::RGBA_16_FLOAT, Texture::SampleCount::SAMPLE_1} };
		imageBasedLightingRenderPassDesc.colorResolveAttachmentLayouts = {};
		imageBasedLightingRenderPassDesc.depthStencilAttachmentLayout = {};
		std::shared_ptr<RenderPass> imageBasedLightingRenderPass = RenderingAPI::CreateRenderPass(imageBasedLightingRenderPassDesc);

		// RENDER HDR IMAGE TO CUBE MAP
		uint32_t environmentCubeMapSize = 1024;

		Texture::Description environmentCubeMapDesc = {};
		environmentCubeMapDesc.type = Texture::Type::TEXTURE_CUBE_MAP;
		environmentCubeMapDesc.width = environmentCubeMapSize;
		environmentCubeMapDesc.height = environmentCubeMapSize;
		environmentCubeMapDesc.format = Texture::Format::RGBA_16_FLOAT;
		environmentCubeMapDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		environmentCubeMapDesc.useAsAttachment = true;
		environmentCubeMapDesc.useSampler = true;
		environmentCubeMapDesc.sampler.isMipMappingEnabled = false;
		environmentCubeMapDesc.sampler.isAnisotropicFilteringEnabled = true;
		environmentCubeMapDesc.sampler.maxAnisotropy = 16;
		environmentCubeMapDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		environmentCubeMapDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		environmentCubeMapDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		m_environmentCubeMap = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(environmentCubeMapDesc));

		std::vector<std::shared_ptr<FrameBuffer>> environmentCubeMapFrameBuffers;
		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			FrameBuffer::Attachment colorAttachment;
			colorAttachment.texture = m_environmentCubeMap;
			colorAttachment.arrayLayer = cubeFaceIndex;

			FrameBuffer::Description frameBufferDesc = {};
			frameBufferDesc.width = environmentCubeMapSize;
			frameBufferDesc.height = environmentCubeMapSize;
			frameBufferDesc.colorAttachments = { colorAttachment };
			frameBufferDesc.colorResolveAttachments = {};
			frameBufferDesc.depthStencilAttachment = {};
			std::shared_ptr<FrameBuffer> frameBuffer = RenderingAPI::CreateFrameBuffer(frameBufferDesc);
			environmentCubeMapFrameBuffers.push_back(frameBuffer);
		}

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

		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			imageBasedLightingRenderPass->Begin(environmentCubeMapFrameBuffers[cubeFaceIndex]);

			hdrImageToCubeMapShader->Bind();
			hdrImageToCubeMapShader->SetUniform("projection", captureProjection);
			hdrImageToCubeMapShader->SetUniform("view", captureViews[cubeFaceIndex]);

			hdrImageToCubeMapShader->SetUniform("hdrTexture", 0);
			hdrTexture->Bind(0);

			// render Cube
			glBindVertexArray(m_cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			imageBasedLightingRenderPass->End();
		}

		// COMPUTE IRRADIANCE CUBEMAP
		uint32_t irradianceCubeMapSize = 64;

		Texture::Description irradianceCubeMapDesc = {};
		irradianceCubeMapDesc.type = Texture::Type::TEXTURE_CUBE_MAP;
		irradianceCubeMapDesc.width = irradianceCubeMapSize;
		irradianceCubeMapDesc.height = irradianceCubeMapSize;
		irradianceCubeMapDesc.format = Texture::Format::RGBA_16_FLOAT;
		irradianceCubeMapDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		irradianceCubeMapDesc.useAsAttachment = true;
		irradianceCubeMapDesc.useSampler = true;
		irradianceCubeMapDesc.sampler.isMipMappingEnabled = false;
		irradianceCubeMapDesc.sampler.isAnisotropicFilteringEnabled = true;
		irradianceCubeMapDesc.sampler.maxAnisotropy = 16;
		irradianceCubeMapDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		irradianceCubeMapDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		irradianceCubeMapDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		m_irradianceCubeMap = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(irradianceCubeMapDesc));

		std::vector<std::shared_ptr<FrameBuffer>> irradianceCubeMapFrameBuffers;
		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			FrameBuffer::Attachment colorAttachment;
			colorAttachment.texture = m_irradianceCubeMap;
			colorAttachment.arrayLayer = cubeFaceIndex;

			FrameBuffer::Description frameBufferDesc = {};
			frameBufferDesc.width = irradianceCubeMapSize;
			frameBufferDesc.height = irradianceCubeMapSize;
			frameBufferDesc.colorAttachments = { colorAttachment };
			frameBufferDesc.colorResolveAttachments = {};
			frameBufferDesc.depthStencilAttachment = {};
			std::shared_ptr<FrameBuffer> frameBuffer = RenderingAPI::CreateFrameBuffer(frameBufferDesc);
			irradianceCubeMapFrameBuffers.push_back(frameBuffer);
		}

		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			imageBasedLightingRenderPass->Begin(irradianceCubeMapFrameBuffers[cubeFaceIndex]);

			irradianceCubeMapShader->Bind();
			irradianceCubeMapShader->SetUniform("projection", captureProjection);
			irradianceCubeMapShader->SetUniform("view", captureViews[cubeFaceIndex]);

			irradianceCubeMapShader->SetUniform("environmentMap", 0);
			m_environmentCubeMap->Bind(0);

			// render Cube
			glBindVertexArray(m_cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			imageBasedLightingRenderPass->End();
		}

		// COMPUTE PREFILTERED CUBEMAP
		uint32_t prefilterCubeMapSize = 256;
		uint32_t maxMipMapLevels = 5;

		Texture::Description prefilterCubeMapDesc = {};
		prefilterCubeMapDesc.type = Texture::Type::TEXTURE_CUBE_MAP;
		prefilterCubeMapDesc.width = prefilterCubeMapSize;
		prefilterCubeMapDesc.height = prefilterCubeMapSize;
		prefilterCubeMapDesc.format = Texture::Format::RGBA_16_FLOAT;
		prefilterCubeMapDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		prefilterCubeMapDesc.useAsAttachment = true;
		prefilterCubeMapDesc.useSampler = true;
		prefilterCubeMapDesc.sampler.isMipMappingEnabled = true;
		prefilterCubeMapDesc.sampler.isAnisotropicFilteringEnabled = true;
		prefilterCubeMapDesc.sampler.maxAnisotropy = 16;
		prefilterCubeMapDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		prefilterCubeMapDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		prefilterCubeMapDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		prefilterCubeMapDesc.sampler.mipMapFilterMode = Texture::FilterMode::LINEAR;
		m_prefilterCubeMap = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(prefilterCubeMapDesc));

		std::vector<std::shared_ptr<FrameBuffer>> prefilterCubeMapFrameBuffers;
		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			for (size_t mipMapLevel = 0; mipMapLevel < maxMipMapLevels; mipMapLevel++)
			{
				FrameBuffer::Attachment colorAttachment;
				colorAttachment.texture = m_prefilterCubeMap;
				colorAttachment.arrayLayer = cubeFaceIndex;
				colorAttachment.mipMapLevel = mipMapLevel;

				FrameBuffer::Description frameBufferDesc = {};
				frameBufferDesc.width = prefilterCubeMapSize * std::pow(0.5, mipMapLevel);
				frameBufferDesc.height = prefilterCubeMapSize * std::pow(0.5, mipMapLevel);
				frameBufferDesc.colorAttachments = { colorAttachment };
				frameBufferDesc.colorResolveAttachments = {};
				frameBufferDesc.depthStencilAttachment = {};
				std::shared_ptr<FrameBuffer> frameBuffer = RenderingAPI::CreateFrameBuffer(frameBufferDesc);
				prefilterCubeMapFrameBuffers.push_back(frameBuffer);
			}
		}

		for (size_t cubeFaceIndex = 0; cubeFaceIndex < 6; cubeFaceIndex++)
		{
			for (size_t mipMapLevel = 0; mipMapLevel < maxMipMapLevels; mipMapLevel++)
			{
				imageBasedLightingRenderPass->Begin(prefilterCubeMapFrameBuffers[mipMapLevel + cubeFaceIndex * maxMipMapLevels]);

				prefilterCubeMapShader->Bind();
				prefilterCubeMapShader->SetUniform("projection", captureProjection);
				prefilterCubeMapShader->SetUniform("view", captureViews[cubeFaceIndex]);

				float roughness = (float)mipMapLevel / (float)(maxMipMapLevels - 1);
				prefilterCubeMapShader->SetUniform("roughness", roughness);

				prefilterCubeMapShader->SetUniform("environmentMap", 0);
				m_environmentCubeMap->Bind(0);

				// render Cube
				glBindVertexArray(m_cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				imageBasedLightingRenderPass->End();
			}
		}

		// COMPUTE BRDF LUT
		uint32_t brdfLUTSize = 1024;

		Texture::Description brdfLUTDesc = {};
		brdfLUTDesc.type = Texture::Type::TEXTURE_2D;
		brdfLUTDesc.width = brdfLUTSize;
		brdfLUTDesc.height = brdfLUTSize;
		brdfLUTDesc.format = Texture::Format::RGBA_16_FLOAT;
		brdfLUTDesc.sampleCount = Texture::SampleCount::SAMPLE_1;
		brdfLUTDesc.useAsAttachment = true;
		brdfLUTDesc.useSampler = true;
		brdfLUTDesc.sampler.isMipMappingEnabled = false;
		brdfLUTDesc.sampler.isAnisotropicFilteringEnabled = true;
		brdfLUTDesc.sampler.maxAnisotropy = 16;
		brdfLUTDesc.sampler.wrapMode = Texture::WrapMode::CLAMP_TO_EDGE;
		brdfLUTDesc.sampler.magnificationFilterMode = Texture::FilterMode::LINEAR;
		brdfLUTDesc.sampler.minificationFilterMode = Texture::FilterMode::LINEAR;
		m_brdfLUT = std::dynamic_pointer_cast<OpenGLTexture>(RenderingAPI::CreateTexture(brdfLUTDesc));

		FrameBuffer::Attachment colorAttachment;
		colorAttachment.texture = m_brdfLUT;

		FrameBuffer::Description frameBufferDesc = {};
		frameBufferDesc.width = brdfLUTSize;
		frameBufferDesc.height = brdfLUTSize;
		frameBufferDesc.colorAttachments = { colorAttachment };
		frameBufferDesc.colorResolveAttachments = {};
		frameBufferDesc.depthStencilAttachment = {};
		std::shared_ptr<FrameBuffer> brdfLUTFrameBuffer = RenderingAPI::CreateFrameBuffer(frameBufferDesc);

		imageBasedLightingRenderPass->Begin(brdfLUTFrameBuffer);

		brdfLUTShader->Bind();

		// render quad
		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		imageBasedLightingRenderPass->End();

		// CLEAN UP
		brdfLUTFrameBuffer->Destroy();
		for (auto frameBuffer : prefilterCubeMapFrameBuffers)
			frameBuffer->Destroy();
		for (auto frameBuffer : irradianceCubeMapFrameBuffers)
			frameBuffer->Destroy();
		for (auto frameBuffer : environmentCubeMapFrameBuffers)
			frameBuffer->Destroy();

		imageBasedLightingRenderPass->Destroy();

		hdrTexture->Destroy();

		brdfLUTShader->Destroy();
		prefilterCubeMapShader->Destroy();
		irradianceCubeMapShader->Destroy();
		hdrImageToCubeMapShader->Destroy();
	}

	void OpenGLRenderer::DestroyPBRShaderResources()
	{
		m_brdfLUT->Destroy();
		m_prefilterCubeMap->Destroy();
		m_irradianceCubeMap->Destroy();
		m_environmentCubeMap->Destroy();
	}
}