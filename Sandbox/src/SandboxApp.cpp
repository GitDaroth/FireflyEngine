#include <FireflyEngine.h>

#include "CameraController.h"

#include <glm/gtc/matrix_transform.hpp>

class SandboxApp : public Firefly::Application
{
public:
	SandboxApp()
	{
		m_renderer = std::make_shared<Firefly::Renderer>();

		m_camera = std::make_shared<Firefly::Camera>(m_window->GetWidth(), m_window->GetHeight());
		m_camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));
		m_cameraController = std::make_shared<CameraController>(m_camera);

		std::shared_ptr<Firefly::Shader> shader = Firefly::RenderingAPI::CreateShader();
		shader->Init("assets/shaders/pbr.glsl");

		std::shared_ptr<Firefly::Mesh> pistolMesh = std::make_shared<Firefly::Mesh>("assets/meshes/pistol.fbx");

		std::shared_ptr<Firefly::Texture2D> pistolAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolAlbedoTexture->Init("assets/textures/pistol_albedo.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolNormalTexture->Init("assets/textures/pistol_normal.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolRoughnessTexture->Init("assets/textures/pistol_roughness.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolMetalnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolMetalnessTexture->Init("assets/textures/pistol_metalness.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolOcclusionTexture->Init("assets/textures/pistol_occlusion.jpg");

		m_pistolMaterial = std::make_shared<Firefly::Material>(shader);
		m_pistolMaterial->SetAlbedoMap(pistolAlbedoTexture);
		m_pistolMaterial->SetNormalMap(pistolNormalTexture);
		m_pistolMaterial->SetRoughnessMap(pistolRoughnessTexture);
		m_pistolMaterial->SetMetalnessMap(pistolMetalnessTexture);
		m_pistolMaterial->SetOcclusionMap(pistolOcclusionTexture);

		m_pistolModel = std::make_shared<Firefly::Model>(pistolMesh, m_pistolMaterial);
		m_pistolModel->SetModelMatrix(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));


		std::vector<Firefly::Mesh::Vertex> vertices = {
			{ {-1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f} },
			{ { 1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f} },
			{ { 1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 1.f} },
			{ {-1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 1.f} }
		};
		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Firefly::Mesh> floorMesh = std::make_shared<Firefly::Mesh>(vertices, indices);

		std::shared_ptr<Firefly::Texture2D> floorAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorAlbedoTexture->Init("assets/textures/floor_albedo.jpg");
		std::shared_ptr<Firefly::Texture2D> floorNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorNormalTexture->Init("assets/textures/floor_normal.jpg");
		std::shared_ptr<Firefly::Texture2D> floorRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorRoughnessTexture->Init("assets/textures/floor_roughness.jpg");
		std::shared_ptr<Firefly::Texture2D> floorOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorOcclusionTexture->Init("assets/textures/floor_occlusion.jpg");
		std::shared_ptr<Firefly::Texture2D> floorHeightTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorHeightTexture->Init("assets/textures/floor_height.jpg");

		m_floorMaterial = std::make_shared<Firefly::Material>(shader);
		m_floorMaterial->SetAlbedoMap(floorAlbedoTexture);
		m_floorMaterial->SetNormalMap(floorNormalTexture);
		m_floorMaterial->SetRoughnessMap(floorRoughnessTexture);
		m_floorMaterial->SetOcclusionMap(floorOcclusionTexture);
		m_floorMaterial->SetHeightMap(floorHeightTexture);
		m_floorMaterial->SetHeightScale(m_heightScale);

		m_floorModel = std::make_shared<Firefly::Model>(floorMesh, m_floorMaterial);
		m_floorModel->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 0.f)), glm::vec3(2.f)));
	}

	~SandboxApp()
	{
	}

protected:
	virtual void OnUpdate(float deltaTime) override
	{
		m_cameraController->OnUpdate(deltaTime);

		Firefly::RenderingAPI::GetRenderFunctions()->SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		Firefly::RenderingAPI::GetRenderFunctions()->Clear();

		m_renderer->BeginScene(m_camera);
		m_renderer->SubmitDraw(m_floorModel);
		m_renderer->SubmitDraw(m_pistolModel);
		m_renderer->EndScene();
	}

	virtual void OnWindowEvent(std::shared_ptr<Firefly::WindowEvent> event) override
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

	virtual void OnKeyEvent(std::shared_ptr<Firefly::KeyEvent> event) override
	{
		if (event->IsType<Firefly::KeyPressEvent>() || event->IsType<Firefly::KeyRepeatEvent>())
		{
			switch (event->GetKeyCode())
			{
			case FIREFLY_KEY_1:
				m_pistolMaterial->EnableAlbedoMap(!m_pistolMaterial->IsAlbedoMapEnabled());
				m_floorMaterial->EnableAlbedoMap(!m_floorMaterial->IsAlbedoMapEnabled());
				break;
			case FIREFLY_KEY_2:
				m_pistolMaterial->EnableNormalMap(!m_pistolMaterial->IsNormalMapEnabled());
				m_floorMaterial->EnableNormalMap(!m_floorMaterial->IsNormalMapEnabled());
				break;
			case FIREFLY_KEY_3:
				m_pistolMaterial->EnableRoughnessMap(!m_pistolMaterial->IsRoughnessMapEnabled());
				m_floorMaterial->EnableRoughnessMap(!m_floorMaterial->IsRoughnessMapEnabled());
				break;
			case FIREFLY_KEY_4:
				m_pistolMaterial->EnableMetalnessMap(!m_pistolMaterial->IsMetalnessMapEnabled());
				m_floorMaterial->EnableMetalnessMap(!m_floorMaterial->IsMetalnessMapEnabled());
				break;
			case FIREFLY_KEY_5:
				m_pistolMaterial->EnableOcclusionMap(!m_pistolMaterial->IsOcclusionMapEnabled());
				m_floorMaterial->EnableOcclusionMap(!m_floorMaterial->IsOcclusionMapEnabled());
				break;
			case FIREFLY_KEY_6:
				m_pistolMaterial->EnableHeightMap(!m_pistolMaterial->IsHeightMapEnabled());
				m_floorMaterial->EnableHeightMap(!m_floorMaterial->IsHeightMapEnabled());
				break;
			case FIREFLY_KEY_UP:
				m_heightScale += 0.01f;
				m_floorMaterial->SetHeightScale(m_heightScale);
				break;
			case FIREFLY_KEY_DOWN:
				m_heightScale = std::max(m_heightScale - 0.01f, 0.f);
				m_floorMaterial->SetHeightScale(m_heightScale);
				break;
			default:
				break;
			}
		}
	}

	virtual void OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event) override
	{
		m_cameraController->OnMouseEvent(event);
	}

	std::shared_ptr<Firefly::Renderer> m_renderer;
	std::shared_ptr<Firefly::Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;

	std::shared_ptr<Firefly::Model> m_pistolModel;
	std::shared_ptr<Firefly::Material> m_pistolMaterial;

	std::shared_ptr<Firefly::Model> m_floorModel;
	std::shared_ptr<Firefly::Material> m_floorMaterial;
	float m_heightScale = 0.2f;
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}