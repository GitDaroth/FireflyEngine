#include "SandboxApp.h"

#include <Firefly/Scene/Components/TagComponent.h>
#include <Firefly/Scene/Components/MeshComponent.h>
#include <Firefly/Scene/Components/MaterialComponent.h>
#include <Firefly/Scene/Components/TransformComponent.h>
#include <glm/gtc/matrix_transform.hpp>

SandboxApp::SandboxApp()
{
	m_renderer = Firefly::RenderingAPI::CreateRenderer();
	m_renderer->Init(m_graphicsContext);

	//m_scene = std::make_shared<Firefly::Scene>();

	m_camera = std::make_shared<Firefly::Camera>(m_window->GetWidth(), m_window->GetHeight());
	m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));
	m_cameraController = std::make_shared<CameraController>(m_camera);

	//std::shared_ptr<Firefly::Shader> shader = Firefly::RenderingAPI::CreateShader();
	//shader->Init("assets/shaders/pbr.glsl");

	//std::vector<Firefly::Mesh::Vertex> vertices = 
	//{
	//	{ {-1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f} },
	//	{ { 1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {2.f, 0.f} },
	//	{ { 1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {2.f, 2.f} },
	//	{ {-1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 2.f} }
	//};
	//std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
	//std::shared_ptr<Firefly::Mesh> floorMesh = std::make_shared<Firefly::Mesh>(vertices, indices);
	//std::shared_ptr<Firefly::Mesh> pistolMesh = std::make_shared<Firefly::Mesh>("assets/meshes/pistol.fbx", true);
	//std::shared_ptr<Firefly::Mesh> globeMesh = std::make_shared<Firefly::Mesh>("assets/meshes/globe.fbx");
	//std::shared_ptr<Firefly::Mesh> armchairMesh = std::make_shared<Firefly::Mesh>("assets/meshes/armchair.fbx");

	//std::shared_ptr<Firefly::Texture2D> pistolAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
	//pistolAlbedoTexture->Init("assets/textures/pistol/albedo.jpg");
	//std::shared_ptr<Firefly::Texture2D> pistolNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
	//pistolNormalTexture->Init("assets/textures/pistol/normal.jpg");
	//std::shared_ptr<Firefly::Texture2D> pistolRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//pistolRoughnessTexture->Init("assets/textures/pistol/roughness.jpg");
	//std::shared_ptr<Firefly::Texture2D> pistolMetalnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//pistolMetalnessTexture->Init("assets/textures/pistol/metalness.jpg");
	//std::shared_ptr<Firefly::Texture2D> pistolOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
	//pistolOcclusionTexture->Init("assets/textures/pistol/occlusion.jpg");
	//std::shared_ptr<Firefly::Material> pistolMaterial = std::make_shared<Firefly::Material>(shader);
	//pistolMaterial->SetAlbedoMap(pistolAlbedoTexture);
	//pistolMaterial->SetNormalMap(pistolNormalTexture);
	//pistolMaterial->SetRoughnessMap(pistolRoughnessTexture);
	//pistolMaterial->SetMetalnessMap(pistolMetalnessTexture);
	//pistolMaterial->SetOcclusionMap(pistolOcclusionTexture);

	//std::shared_ptr<Firefly::Texture2D> globeAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
	//globeAlbedoTexture->Init("assets/textures/globe/albedo.png");
	//std::shared_ptr<Firefly::Texture2D> globeRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//globeRoughnessTexture->Init("assets/textures/globe/roughness.png");
	//std::shared_ptr<Firefly::Texture2D> globeMetalnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//globeMetalnessTexture->Init("assets/textures/globe/metalness.png");
	//std::shared_ptr<Firefly::Texture2D> globeOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
	//globeOcclusionTexture->Init("assets/textures/globe/occlusion.png");
	//std::shared_ptr<Firefly::Material> globeMaterial = std::make_shared<Firefly::Material>(shader);
	//globeMaterial->SetAlbedoMap(globeAlbedoTexture);
	//globeMaterial->SetRoughnessMap(globeRoughnessTexture);
	//globeMaterial->SetMetalnessMap(globeMetalnessTexture);
	//globeMaterial->SetOcclusionMap(globeOcclusionTexture);

	//std::shared_ptr<Firefly::Texture2D> armchairAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
	//armchairAlbedoTexture->Init("assets/textures/armchair/albedo.png");
	//std::shared_ptr<Firefly::Texture2D> armchairRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//armchairRoughnessTexture->Init("assets/textures/armchair/roughness.png");
	//std::shared_ptr<Firefly::Texture2D> armchairNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
	//armchairNormalTexture->Init("assets/textures/armchair/normal.png");
	//std::shared_ptr<Firefly::Texture2D> armchairOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
	//armchairOcclusionTexture->Init("assets/textures/armchair/occlusion.png");
	//std::shared_ptr<Firefly::Material> armchairMaterial = std::make_shared<Firefly::Material>(shader);
	//armchairMaterial->SetAlbedoMap(armchairAlbedoTexture);
	//armchairMaterial->SetRoughnessMap(armchairRoughnessTexture);
	//armchairMaterial->SetNormalMap(armchairNormalTexture);
	//armchairMaterial->SetOcclusionMap(armchairOcclusionTexture);

	//std::shared_ptr<Firefly::Texture2D> floorAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floorAlbedoTexture->Init("assets/textures/floor/1_albedo.jpg");
	//std::shared_ptr<Firefly::Texture2D> floorNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floorNormalTexture->Init("assets/textures/floor/1_normal.jpg");
	//std::shared_ptr<Firefly::Texture2D> floorRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floorRoughnessTexture->Init("assets/textures/floor/1_roughness.jpg");
	//std::shared_ptr<Firefly::Texture2D> floorOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floorOcclusionTexture->Init("assets/textures/floor/1_occlusion.jpg");
	//std::shared_ptr<Firefly::Texture2D> floorHeightTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floorHeightTexture->Init("assets/textures/floor/1_height.jpg");
	//std::shared_ptr<Firefly::Material> floorMaterial = std::make_shared<Firefly::Material>(shader);
	//floorMaterial->SetAlbedoMap(floorAlbedoTexture);
	//floorMaterial->SetNormalMap(floorNormalTexture);
	//floorMaterial->SetRoughnessMap(floorRoughnessTexture);
	//floorMaterial->SetOcclusionMap(floorOcclusionTexture);
	//floorMaterial->SetHeightMap(floorHeightTexture);
	//floorMaterial->SetHeightScale(m_heightScale);

	//std::shared_ptr<Firefly::Texture2D> floor2AlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floor2AlbedoTexture->Init("assets/textures/floor/2_albedo.jpg");
	//std::shared_ptr<Firefly::Texture2D> floor2NormalTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floor2NormalTexture->Init("assets/textures/floor/2_normal.jpg");
	//std::shared_ptr<Firefly::Texture2D> floor2RoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
	//floor2RoughnessTexture->Init("assets/textures/floor/2_roughness.jpg");
	//std::shared_ptr<Firefly::Material> floor2Material = std::make_shared<Firefly::Material>(shader);
	//floor2Material->SetAlbedoMap(floor2AlbedoTexture);
	//floor2Material->SetNormalMap(floor2NormalTexture);
	//floor2Material->SetRoughnessMap(floor2RoughnessTexture);

	//Firefly::Entity pistol(m_scene);
	//pistol.AddComponent<Firefly::TagComponent>("Pistol");
	//pistol.AddComponent<Firefly::TransformComponent>(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));
	//pistol.AddComponent<Firefly::MeshComponent>(pistolMesh);
	//pistol.AddComponent<Firefly::MaterialComponent>(pistolMaterial);

	//Firefly::Entity globe(m_scene);
	//globe.AddComponent<Firefly::TagComponent>("Globe");
	//globe.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5f, -0.5f, -1.5f)), glm::vec3(0.0075f)));
	//globe.AddComponent<Firefly::MeshComponent>(globeMesh);
	//globe.AddComponent<Firefly::MaterialComponent>(globeMaterial);

	//Firefly::Entity armchair(m_scene);
	//armchair.AddComponent<Firefly::TagComponent>("Armchair");
	//armchair.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::rotate(glm::mat4(1), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.5f, 1.5f, -0.55f)), glm::vec3(0.01f)));
	//armchair.AddComponent<Firefly::MeshComponent>(armchairMesh);
	//armchair.AddComponent<Firefly::MaterialComponent>(armchairMaterial);

	//Firefly::Entity floor(m_scene);
	//floor.AddComponent<Firefly::TagComponent>("Floor");
	//floor.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 8.f)), glm::vec3(4.f)));
	//floor.AddComponent<Firefly::MeshComponent>(floorMesh);
	//floor.AddComponent<Firefly::MaterialComponent>(floorMaterial);

	//Firefly::Entity floor2(m_scene);
	//floor2.AddComponent<Firefly::TagComponent>("Floor2");
	//floor2.AddComponent<Firefly::TransformComponent>(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 0.f)), glm::vec3(4.f)));
	//floor2.AddComponent<Firefly::MeshComponent>(floorMesh);
	//floor2.AddComponent<Firefly::MaterialComponent>(floor2Material);
}

SandboxApp::~SandboxApp()
{
	m_renderer->Destroy();
}

void SandboxApp::OnUpdate(float deltaTime)
{
	m_cameraController->OnUpdate(deltaTime);

	//m_renderer->BeginDrawRecording();
	//m_renderer->RecordDraw(Object1);
	//m_renderer->RecordDraw(Object2);
	//m_renderer->RecordDraw(Object3);
	//m_renderer->EndDrawRecording();
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
	//if (event->IsType<Firefly::KeyPressEvent>() || event->IsType<Firefly::KeyRepeatEvent>())
	//{
	//	switch (event->GetKeyCode())
	//	{
	//	case FIREFLY_KEY_UP:
	//		m_heightScale = std::max(m_heightScale - 0.01f, 0.f);
	//		break;
	//	case FIREFLY_KEY_DOWN:
	//		m_heightScale += 0.01f;
	//		break;
	//	}

	//	auto entityGroup = m_scene->GetEntityGroup<Firefly::MaterialComponent>();
	//	for (auto entity : entityGroup)
	//	{
	//		auto material = entity.GetComponent<Firefly::MaterialComponent>().m_material;
	//		
	//		switch (event->GetKeyCode())
	//		{
	//		case FIREFLY_KEY_1:
	//			material->EnableAlbedoMap(!material->IsAlbedoMapEnabled());
	//			break;
	//		case FIREFLY_KEY_2:
	//			material->EnableNormalMap(!material->IsNormalMapEnabled());
	//			break;
	//		case FIREFLY_KEY_3:
	//			material->EnableRoughnessMap(!material->IsRoughnessMapEnabled());
	//			break;
	//		case FIREFLY_KEY_4:
	//			material->EnableMetalnessMap(!material->IsMetalnessMapEnabled());
	//			break;
	//		case FIREFLY_KEY_5:
	//			material->EnableOcclusionMap(!material->IsOcclusionMapEnabled());
	//			break;
	//		case FIREFLY_KEY_6:
	//			material->EnableHeightMap(!material->IsHeightMapEnabled());
	//			break;
	//		case FIREFLY_KEY_UP:
	//			material->SetHeightScale(m_heightScale);
	//			break;
	//		case FIREFLY_KEY_DOWN:
	//			material->SetHeightScale(m_heightScale);
	//			break;
	//		default:
	//			break;
	//		}
	//	}
	//}
}

void SandboxApp::OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event)
{
	m_cameraController->OnMouseEvent(event);
}

void SandboxApp::OnGamepadEvent(std::shared_ptr<Firefly::GamepadEvent> event)
{
}