#include "SandboxApp.h"

#include <Firefly/Scene/Components/TagComponent.h>
#include <Firefly/Scene/Components/MeshComponent.h>
#include <Firefly/Scene/Components/MaterialComponent.h>
#include <Firefly/Scene/Components/TransformComponent.h>
#include <Firefly/Core/ResourceRegistry.h>
#include <glm/gtc/matrix_transform.hpp>

SandboxApp::SandboxApp()
{
	m_scene = std::make_shared<Firefly::Scene>();
	m_camera = std::make_shared<Firefly::Camera>(m_window->GetWidth(), m_window->GetHeight());
	m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));
	m_cameraController = std::make_shared<CameraController>(m_camera);

	Firefly::ShaderCode shaderCode{};
	shaderCode.vertex = Firefly::Shader::ReadShaderCodeFromFile("assets/shaders/triangle.vert.spv");
	shaderCode.fragment = Firefly::Shader::ReadShaderCodeFromFile("assets/shaders/triangle.frag.spv");
	std::shared_ptr<Firefly::Shader> defaultShader = Firefly::RenderingAPI::CreateShader(m_graphicsContext);
	defaultShader->Init("Lit", shaderCode);
	Firefly::ShaderRegistry::Instance().Insert(defaultShader->GetTag(), defaultShader);

	std::vector<Firefly::Mesh::Vertex> vertices = 
	{
		{ {-1.f, 0.f,  1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, -1.f, 0.f}, {0.f, 0.f} },
		{ { 1.f, 0.f,  1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, -1.f, 0.f}, {2.f, 0.f} },
		{ { 1.f, 0.f, -1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, -1.f, 0.f}, {2.f, 2.f} },
		{ {-1.f, 0.f, -1.f, 1.f}, {0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, -1.f, 0.f}, {0.f, 2.f} }
	};
	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	std::shared_ptr<Firefly::Mesh> floorMesh = Firefly::RenderingAPI::CreateMesh(m_graphicsContext);
	floorMesh->Init(vertices, indices);
	std::shared_ptr<Firefly::Mesh> pistolMesh = Firefly::RenderingAPI::CreateMesh(m_graphicsContext);
	pistolMesh->Init("assets/meshes/pistol.fbx", true);
	std::shared_ptr<Firefly::Mesh> globeMesh = Firefly::RenderingAPI::CreateMesh(m_graphicsContext);
	globeMesh->Init("assets/meshes/globe.fbx");
	std::shared_ptr<Firefly::Mesh> armchairMesh = Firefly::RenderingAPI::CreateMesh(m_graphicsContext);
	armchairMesh->Init("assets/meshes/armchair.fbx");

	Firefly::MeshRegistry::Instance().Insert("Floor", floorMesh);
	Firefly::MeshRegistry::Instance().Insert("Pistol", pistolMesh);
	Firefly::MeshRegistry::Instance().Insert("Globe", globeMesh);
	Firefly::MeshRegistry::Instance().Insert("Armchair", armchairMesh);

	std::shared_ptr<Firefly::Texture> pistolAlbedoTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	pistolAlbedoTexture->Init("assets/textures/pistol/albedo.jpg");
	std::shared_ptr<Firefly::Texture> pistolNormalTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	pistolNormalTexture->Init("assets/textures/pistol/normal.jpg");
	std::shared_ptr<Firefly::Texture> pistolRoughnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	pistolRoughnessTexture->Init("assets/textures/pistol/roughness.jpg");
	std::shared_ptr<Firefly::Texture> pistolMetalnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	pistolMetalnessTexture->Init("assets/textures/pistol/metalness.jpg");
	std::shared_ptr<Firefly::Texture> pistolOcclusionTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	pistolOcclusionTexture->Init("assets/textures/pistol/occlusion.jpg");
	Firefly::TextureRegistry::Instance().Insert("PistolAlbedo", pistolAlbedoTexture);
	Firefly::TextureRegistry::Instance().Insert("PistolNormal", pistolNormalTexture);
	Firefly::TextureRegistry::Instance().Insert("PistolRoughness", pistolRoughnessTexture);
	Firefly::TextureRegistry::Instance().Insert("PistolMetallness", pistolMetalnessTexture);
	Firefly::TextureRegistry::Instance().Insert("PistolOcclusion", pistolOcclusionTexture);

	std::shared_ptr<Firefly::Texture> globeAlbedoTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	globeAlbedoTexture->Init("assets/textures/globe/albedo.png");
	std::shared_ptr<Firefly::Texture> globeRoughnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	globeRoughnessTexture->Init("assets/textures/globe/roughness.png");
	std::shared_ptr<Firefly::Texture> globeMetalnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	globeMetalnessTexture->Init("assets/textures/globe/metalness.png");
	std::shared_ptr<Firefly::Texture> globeOcclusionTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	globeOcclusionTexture->Init("assets/textures/globe/occlusion.png");
	Firefly::TextureRegistry::Instance().Insert("GlobeAlbedo", globeAlbedoTexture);
	Firefly::TextureRegistry::Instance().Insert("GlobeRoughness", globeRoughnessTexture);
	Firefly::TextureRegistry::Instance().Insert("GlobeMetalness", globeMetalnessTexture);
	Firefly::TextureRegistry::Instance().Insert("GlobeOcclusion", globeOcclusionTexture);

	std::shared_ptr<Firefly::Texture> armchairAlbedoTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	armchairAlbedoTexture->Init("assets/textures/armchair/albedo.png");
	std::shared_ptr<Firefly::Texture> armchairNormalTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	armchairNormalTexture->Init("assets/textures/armchair/normal.png");
	std::shared_ptr<Firefly::Texture> armchairRoughnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	armchairRoughnessTexture->Init("assets/textures/armchair/roughness.png");
	std::shared_ptr<Firefly::Texture> armchairOcclusionTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	armchairOcclusionTexture->Init("assets/textures/armchair/occlusion.png");
	Firefly::TextureRegistry::Instance().Insert("ArmchairAlbedo", armchairAlbedoTexture);
	Firefly::TextureRegistry::Instance().Insert("ArmchairNormal", armchairNormalTexture);
	Firefly::TextureRegistry::Instance().Insert("ArmchairRoughness", armchairRoughnessTexture);
	Firefly::TextureRegistry::Instance().Insert("ArmchairOcclusion", armchairOcclusionTexture);

	std::shared_ptr<Firefly::Texture> floorAlbedoTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floorAlbedoTexture->Init("assets/textures/floor/1_albedo.jpg");
	std::shared_ptr<Firefly::Texture> floorNormalTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floorNormalTexture->Init("assets/textures/floor/1_normal.jpg");
	std::shared_ptr<Firefly::Texture> floorRoughnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floorRoughnessTexture->Init("assets/textures/floor/1_roughness.jpg");
	std::shared_ptr<Firefly::Texture> floorOcclusionTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floorOcclusionTexture->Init("assets/textures/floor/1_occlusion.jpg");
	std::shared_ptr<Firefly::Texture> floorHeightTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floorHeightTexture->Init("assets/textures/floor/1_height.jpg");
	Firefly::TextureRegistry::Instance().Insert("FloorAlbedo", floorAlbedoTexture);
	Firefly::TextureRegistry::Instance().Insert("FloorNormal", floorNormalTexture);
	Firefly::TextureRegistry::Instance().Insert("FloorRoughness", floorRoughnessTexture);
	Firefly::TextureRegistry::Instance().Insert("FloorOcclusion", floorOcclusionTexture);
	Firefly::TextureRegistry::Instance().Insert("FloorHeight", floorHeightTexture);

	std::shared_ptr<Firefly::Texture> floor2AlbedoTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floor2AlbedoTexture->Init("assets/textures/floor/2_albedo.jpg");
	std::shared_ptr<Firefly::Texture> floor2NormalTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floor2NormalTexture->Init("assets/textures/floor/2_normal.jpg");
	std::shared_ptr<Firefly::Texture> floor2RoughnessTexture = Firefly::RenderingAPI::CreateTexture(m_graphicsContext);
	floor2RoughnessTexture->Init("assets/textures/floor/2_roughness.jpg");
	Firefly::TextureRegistry::Instance().Insert("Floor2Albedo", floor2AlbedoTexture);
	Firefly::TextureRegistry::Instance().Insert("Floor2Normal", floor2NormalTexture);
	Firefly::TextureRegistry::Instance().Insert("Floor2Roughness", floor2RoughnessTexture);

	std::shared_ptr<Firefly::Material> pistolMaterial = Firefly::RenderingAPI::CreateMaterial(m_graphicsContext);
	pistolMaterial->Init(defaultShader);
	pistolMaterial->SetTexture(pistolAlbedoTexture, Firefly::Material::TextureUsage::Albedo);
	pistolMaterial->SetTexture(pistolNormalTexture, Firefly::Material::TextureUsage::Normal);
	pistolMaterial->SetTexture(pistolRoughnessTexture, Firefly::Material::TextureUsage::Roughness);
	pistolMaterial->SetTexture(pistolMetalnessTexture, Firefly::Material::TextureUsage::Metalness);
	pistolMaterial->SetTexture(pistolOcclusionTexture, Firefly::Material::TextureUsage::Occlusion);

	std::shared_ptr<Firefly::Material> globeMaterial = Firefly::RenderingAPI::CreateMaterial(m_graphicsContext);
	globeMaterial->Init(defaultShader);
	globeMaterial->SetTexture(globeAlbedoTexture, Firefly::Material::TextureUsage::Albedo);
	globeMaterial->SetTexture(globeRoughnessTexture, Firefly::Material::TextureUsage::Roughness);
	globeMaterial->SetTexture(globeMetalnessTexture, Firefly::Material::TextureUsage::Metalness);
	globeMaterial->SetTexture(globeOcclusionTexture, Firefly::Material::TextureUsage::Occlusion);

	std::shared_ptr<Firefly::Material> armchairMaterial = Firefly::RenderingAPI::CreateMaterial(m_graphicsContext);
	armchairMaterial->Init(defaultShader);
	armchairMaterial->SetTexture(armchairAlbedoTexture, Firefly::Material::TextureUsage::Albedo);
	armchairMaterial->SetTexture(armchairNormalTexture, Firefly::Material::TextureUsage::Normal);
	armchairMaterial->SetTexture(armchairRoughnessTexture, Firefly::Material::TextureUsage::Roughness);
	armchairMaterial->SetTexture(armchairOcclusionTexture, Firefly::Material::TextureUsage::Occlusion);

	std::shared_ptr<Firefly::Material> floorMaterial = Firefly::RenderingAPI::CreateMaterial(m_graphicsContext);
	floorMaterial->Init(defaultShader);
	floorMaterial->SetTexture(floorAlbedoTexture, Firefly::Material::TextureUsage::Albedo);
	floorMaterial->SetTexture(floorNormalTexture, Firefly::Material::TextureUsage::Normal);
	floorMaterial->SetTexture(floorRoughnessTexture, Firefly::Material::TextureUsage::Roughness);
	floorMaterial->SetTexture(floorOcclusionTexture, Firefly::Material::TextureUsage::Occlusion);
	floorMaterial->SetTexture(floorHeightTexture, Firefly::Material::TextureUsage::Height);
	floorMaterial->SetHeightScale(m_heightScale);

	std::shared_ptr<Firefly::Material> floor2Material = Firefly::RenderingAPI::CreateMaterial(m_graphicsContext);
	floor2Material->Init(defaultShader);
	floor2Material->SetTexture(floor2AlbedoTexture, Firefly::Material::TextureUsage::Albedo);
	floor2Material->SetTexture(floor2NormalTexture, Firefly::Material::TextureUsage::Normal);
	floor2Material->SetTexture(floor2RoughnessTexture, Firefly::Material::TextureUsage::Roughness);

	Firefly::MaterialRegistry::Instance().Insert("Pistol", pistolMaterial);
	Firefly::MaterialRegistry::Instance().Insert("Globe", globeMaterial);
	Firefly::MaterialRegistry::Instance().Insert("Armchair", armchairMaterial);
	Firefly::MaterialRegistry::Instance().Insert("Floor", floorMaterial);
	Firefly::MaterialRegistry::Instance().Insert("Floor2", floor2Material);

	Firefly::Entity pistol(m_scene);
	pistol.AddComponent<Firefly::TagComponent>("Pistol");
	pistol.AddComponent<Firefly::TransformComponent>(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));
	pistol.AddComponent<Firefly::MeshComponent>(pistolMesh);
	pistol.AddComponent<Firefly::MaterialComponent>(pistolMaterial);

	Firefly::Entity globe(m_scene);
	globe.AddComponent<Firefly::TagComponent>("Globe");
	globe.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5f, -0.5f, -1.5f)), glm::vec3(0.0075f)));
	globe.AddComponent<Firefly::MeshComponent>(globeMesh);
	globe.AddComponent<Firefly::MaterialComponent>(globeMaterial);

	Firefly::Entity armchair(m_scene);
	armchair.AddComponent<Firefly::TagComponent>("Armchair");
	armchair.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::rotate(glm::mat4(1), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.5f, 1.5f, -0.55f)), glm::vec3(0.01f)));
	armchair.AddComponent<Firefly::MeshComponent>(armchairMesh);
	armchair.AddComponent<Firefly::MaterialComponent>(armchairMaterial);

	Firefly::Entity floor(m_scene);
	floor.AddComponent<Firefly::TagComponent>("Floor");
	floor.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 8.f)), glm::vec3(4.f)));
	floor.AddComponent<Firefly::MeshComponent>(floorMesh);
	floor.AddComponent<Firefly::MaterialComponent>(floorMaterial);

	Firefly::Entity floor2(m_scene);
	floor2.AddComponent<Firefly::TagComponent>("Floor2");
	floor2.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 0.f)), glm::vec3(4.f)));
	floor2.AddComponent<Firefly::MeshComponent>(floorMesh);
	floor2.AddComponent<Firefly::MaterialComponent>(floor2Material);

	m_renderer = Firefly::RenderingAPI::CreateRenderer(m_graphicsContext);
	m_renderer->Init();
}

SandboxApp::~SandboxApp()
{
	m_renderer->Destroy();
}

void SandboxApp::OnUpdate(float deltaTime)
{
	m_cameraController->OnUpdate(deltaTime);

	m_renderer->BeginDrawRecording();
	for(auto entity : m_scene->GetEntities())
		m_renderer->RecordDraw(entity);
	m_renderer->EndDrawRecording();
	m_renderer->SubmitDraw(m_camera);
}

void SandboxApp::OnWindowEvent(std::shared_ptr<Firefly::WindowEvent> event)
{
	if (auto resizeEvent = event->AsType<Firefly::WindowResizeEvent>())
	{
		int width = resizeEvent->GetWidth();
		int height = resizeEvent->GetHeight();

		if (width != 0 && height != 0)
		{
			m_camera->SetWidth(width);
			m_camera->SetHeight(height);
		}
	}
}

void SandboxApp::OnKeyEvent(std::shared_ptr<Firefly::KeyEvent> event)
{
	if (event->IsType<Firefly::KeyPressEvent>() || event->IsType<Firefly::KeyRepeatEvent>())
	{
		switch (event->GetKeyCode())
		{
		case FIREFLY_KEY_1:
			m_isAlbedoTexEnabled = !m_isAlbedoTexEnabled;
			break;
		case FIREFLY_KEY_2:
			m_isNormalTexEnabled = !m_isNormalTexEnabled;
			break;
		case FIREFLY_KEY_3:
			m_isRoughnessTexEnabled = !m_isRoughnessTexEnabled;
			break;
		case FIREFLY_KEY_4:
			m_isMetalnessTexEnabled = !m_isMetalnessTexEnabled;
			break;
		case FIREFLY_KEY_5:
			m_isOcclusionTexEnabled = !m_isOcclusionTexEnabled;
			break;
		case FIREFLY_KEY_6:
			m_isHeightTexEnabled = !m_isHeightTexEnabled;
			break;
		case FIREFLY_KEY_UP:
			m_heightScale = std::max(m_heightScale - 0.01f, 0.f);
			break;
		case FIREFLY_KEY_DOWN:
			m_heightScale += 0.01f;
			break;
		}

		auto entityGroup = m_scene->GetEntityGroup<Firefly::MaterialComponent>();
		for (auto entity : entityGroup)
		{
			auto material = entity.GetComponent<Firefly::MaterialComponent>().m_material;
			
			switch (event->GetKeyCode())
			{
			case FIREFLY_KEY_1:
				material->EnableTexture(m_isAlbedoTexEnabled, Firefly::Material::TextureUsage::Albedo);
				break;
			case FIREFLY_KEY_2:
				material->EnableTexture(m_isNormalTexEnabled, Firefly::Material::TextureUsage::Normal);
				break;
			case FIREFLY_KEY_3:
				material->EnableTexture(m_isRoughnessTexEnabled, Firefly::Material::TextureUsage::Roughness);
				break;
			case FIREFLY_KEY_4:
				material->EnableTexture(m_isMetalnessTexEnabled, Firefly::Material::TextureUsage::Metalness);
				break;
			case FIREFLY_KEY_5:
				material->EnableTexture(m_isOcclusionTexEnabled, Firefly::Material::TextureUsage::Occlusion);
				break;
			case FIREFLY_KEY_6:
				material->EnableTexture(m_isHeightTexEnabled, Firefly::Material::TextureUsage::Height);
				break;
			case FIREFLY_KEY_UP:
				material->SetHeightScale(m_heightScale);
				break;
			case FIREFLY_KEY_DOWN:
				material->SetHeightScale(m_heightScale);
				break;
			default:
				break;
			}
		}
	}
}

void SandboxApp::OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event)
{
	m_cameraController->OnMouseEvent(event);
}

void SandboxApp::OnGamepadEvent(std::shared_ptr<Firefly::GamepadEvent> event)
{
}