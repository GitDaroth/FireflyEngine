#include <FireflyEngine.h>

#include "CameraController.h"

#include <glm/gtc/matrix_transform.hpp>
#include <entt.hpp>

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

		std::shared_ptr<Firefly::Mesh> pistolMesh = std::make_shared<Firefly::Mesh>("assets/meshes/pistol.fbx", true);

		std::shared_ptr<Firefly::Texture2D> pistolAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolAlbedoTexture->Init("assets/textures/pistol/albedo.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolNormalTexture->Init("assets/textures/pistol/normal.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolRoughnessTexture->Init("assets/textures/pistol/roughness.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolMetalnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolMetalnessTexture->Init("assets/textures/pistol/metalness.jpg");
		std::shared_ptr<Firefly::Texture2D> pistolOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		pistolOcclusionTexture->Init("assets/textures/pistol/occlusion.jpg");

		m_pistolMaterial = std::make_shared<Firefly::Material>(shader);
		m_pistolMaterial->SetAlbedoMap(pistolAlbedoTexture);
		m_pistolMaterial->SetNormalMap(pistolNormalTexture);
		m_pistolMaterial->SetRoughnessMap(pistolRoughnessTexture);
		m_pistolMaterial->SetMetalnessMap(pistolMetalnessTexture);
		m_pistolMaterial->SetOcclusionMap(pistolOcclusionTexture);

		m_pistolModel = std::make_shared<Firefly::Model>(pistolMesh, m_pistolMaterial);
		m_pistolModel->SetModelMatrix(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));


		std::shared_ptr<Firefly::Mesh> globeMesh = std::make_shared<Firefly::Mesh>("assets/meshes/globe.fbx");

		std::shared_ptr<Firefly::Texture2D> globeAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		globeAlbedoTexture->Init("assets/textures/globe/albedo.png");
		std::shared_ptr<Firefly::Texture2D> globeRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		globeRoughnessTexture->Init("assets/textures/globe/roughness.png");
		std::shared_ptr<Firefly::Texture2D> globeMetalnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		globeMetalnessTexture->Init("assets/textures/globe/metalness.png");
		std::shared_ptr<Firefly::Texture2D> globeOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		globeOcclusionTexture->Init("assets/textures/globe/occlusion.png");

		m_globeMaterial = std::make_shared<Firefly::Material>(shader);
		m_globeMaterial->SetAlbedoMap(globeAlbedoTexture);
		m_globeMaterial->SetRoughnessMap(globeRoughnessTexture);
		m_globeMaterial->SetMetalnessMap(globeMetalnessTexture);
		m_globeMaterial->SetOcclusionMap(globeOcclusionTexture);

		m_globeModel = std::make_shared<Firefly::Model>(globeMesh, m_globeMaterial);
		m_globeModel->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5f, -0.5f, -1.5f)), glm::vec3(0.0075f)));


		std::shared_ptr<Firefly::Mesh> armchairMesh = std::make_shared<Firefly::Mesh>("assets/meshes/armchair.fbx");

		std::shared_ptr<Firefly::Texture2D> armchairAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		armchairAlbedoTexture->Init("assets/textures/armchair/albedo.png");
		std::shared_ptr<Firefly::Texture2D> armchairRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		armchairRoughnessTexture->Init("assets/textures/armchair/roughness.png");
		std::shared_ptr<Firefly::Texture2D> armchairNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		armchairNormalTexture->Init("assets/textures/armchair/normal.png");
		std::shared_ptr<Firefly::Texture2D> armchairOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		armchairOcclusionTexture->Init("assets/textures/armchair/occlusion.png");

		m_armchairMaterial = std::make_shared<Firefly::Material>(shader);
		m_armchairMaterial->SetAlbedoMap(armchairAlbedoTexture);
		m_armchairMaterial->SetRoughnessMap(armchairRoughnessTexture);
		m_armchairMaterial->SetNormalMap(armchairNormalTexture);
		m_armchairMaterial->SetOcclusionMap(armchairOcclusionTexture);

		m_armchairModel = std::make_shared<Firefly::Model>(armchairMesh, m_armchairMaterial);
		m_armchairModel->SetModelMatrix(glm::scale(glm::translate(glm::rotate(glm::mat4(1), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.5f, 1.5f, -0.55f)), glm::vec3(0.01f)));


		std::vector<Firefly::Mesh::Vertex> vertices = {
			{ {-1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f} },
			{ { 1.f, 0.f,  1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {2.f, 0.f} },
			{ { 1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {2.f, 2.f} },
			{ {-1.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 2.f} }
		};
		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Firefly::Mesh> floorMesh = std::make_shared<Firefly::Mesh>(vertices, indices);

		std::shared_ptr<Firefly::Texture2D> floorAlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorAlbedoTexture->Init("assets/textures/floor/1_albedo.jpg");
		std::shared_ptr<Firefly::Texture2D> floorNormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorNormalTexture->Init("assets/textures/floor/1_normal.jpg");
		std::shared_ptr<Firefly::Texture2D> floorRoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorRoughnessTexture->Init("assets/textures/floor/1_roughness.jpg");
		std::shared_ptr<Firefly::Texture2D> floorOcclusionTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorOcclusionTexture->Init("assets/textures/floor/1_occlusion.jpg");
		std::shared_ptr<Firefly::Texture2D> floorHeightTexture = Firefly::RenderingAPI::CreateTexture2D();
		floorHeightTexture->Init("assets/textures/floor/1_height.jpg");

		m_floorMaterial = std::make_shared<Firefly::Material>(shader);
		m_floorMaterial->SetAlbedoMap(floorAlbedoTexture);
		m_floorMaterial->SetNormalMap(floorNormalTexture);
		m_floorMaterial->SetRoughnessMap(floorRoughnessTexture);
		m_floorMaterial->SetOcclusionMap(floorOcclusionTexture);
		m_floorMaterial->SetHeightMap(floorHeightTexture);
		m_floorMaterial->SetHeightScale(m_heightScale);

		m_floorModel = std::make_shared<Firefly::Model>(floorMesh, m_floorMaterial);
		m_floorModel->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 8.f)), glm::vec3(4.f)));


		std::shared_ptr<Firefly::Texture2D> floor2AlbedoTexture = Firefly::RenderingAPI::CreateTexture2D();
		floor2AlbedoTexture->Init("assets/textures/floor/2_albedo.jpg");
		std::shared_ptr<Firefly::Texture2D> floor2NormalTexture = Firefly::RenderingAPI::CreateTexture2D();
		floor2NormalTexture->Init("assets/textures/floor/2_normal.jpg");
		std::shared_ptr<Firefly::Texture2D> floor2RoughnessTexture = Firefly::RenderingAPI::CreateTexture2D();
		floor2RoughnessTexture->Init("assets/textures/floor/2_roughness.jpg");
		std::shared_ptr<Firefly::Texture2D> floor2HeightTexture = Firefly::RenderingAPI::CreateTexture2D();
		floor2HeightTexture->Init("assets/textures/floor/2_height.jpg");

		m_floor2Material = std::make_shared<Firefly::Material>(shader);
		m_floor2Material->SetAlbedoMap(floor2AlbedoTexture);
		m_floor2Material->SetNormalMap(floor2NormalTexture);
		m_floor2Material->SetRoughnessMap(floor2RoughnessTexture);
		m_floor2Material->SetHeightMap(floor2HeightTexture);
		m_floor2Material->SetHeightScale(m_heightScale);

		m_floor2Model = std::make_shared<Firefly::Model>(floorMesh, m_floor2Material);
		m_floor2Model->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.f, -0.5f, 0.f)), glm::vec3(4.f)));
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
		m_renderer->SubmitDraw(m_floor2Model);
		m_renderer->SubmitDraw(m_pistolModel);
		m_renderer->SubmitDraw(m_globeModel);
		m_renderer->SubmitDraw(m_armchairModel);
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
				m_globeMaterial->EnableAlbedoMap(!m_globeMaterial->IsAlbedoMapEnabled());
				m_armchairMaterial->EnableAlbedoMap(!m_armchairMaterial->IsAlbedoMapEnabled());
				m_floorMaterial->EnableAlbedoMap(!m_floorMaterial->IsAlbedoMapEnabled());
				m_floor2Material->EnableAlbedoMap(!m_floor2Material->IsAlbedoMapEnabled());
				break;
			case FIREFLY_KEY_2:
				m_pistolMaterial->EnableNormalMap(!m_pistolMaterial->IsNormalMapEnabled());
				m_globeMaterial->EnableNormalMap(!m_globeMaterial->IsNormalMapEnabled());
				m_armchairMaterial->EnableNormalMap(!m_armchairMaterial->IsNormalMapEnabled());
				m_floorMaterial->EnableNormalMap(!m_floorMaterial->IsNormalMapEnabled());
				m_floor2Material->EnableNormalMap(!m_floor2Material->IsNormalMapEnabled());
				break;
			case FIREFLY_KEY_3:
				m_pistolMaterial->EnableRoughnessMap(!m_pistolMaterial->IsRoughnessMapEnabled());
				m_globeMaterial->EnableRoughnessMap(!m_globeMaterial->IsRoughnessMapEnabled());
				m_armchairMaterial->EnableRoughnessMap(!m_armchairMaterial->IsRoughnessMapEnabled());
				m_floorMaterial->EnableRoughnessMap(!m_floorMaterial->IsRoughnessMapEnabled());
				m_floor2Material->EnableRoughnessMap(!m_floor2Material->IsRoughnessMapEnabled());
				break;
			case FIREFLY_KEY_4:
				m_pistolMaterial->EnableMetalnessMap(!m_pistolMaterial->IsMetalnessMapEnabled());
				m_globeMaterial->EnableMetalnessMap(!m_globeMaterial->IsMetalnessMapEnabled());
				m_armchairMaterial->EnableMetalnessMap(!m_armchairMaterial->IsMetalnessMapEnabled());
				m_floorMaterial->EnableMetalnessMap(!m_floorMaterial->IsMetalnessMapEnabled());
				m_floor2Material->EnableMetalnessMap(!m_floor2Material->IsMetalnessMapEnabled());
				break;
			case FIREFLY_KEY_5:
				m_pistolMaterial->EnableOcclusionMap(!m_pistolMaterial->IsOcclusionMapEnabled());
				m_globeMaterial->EnableOcclusionMap(!m_globeMaterial->IsOcclusionMapEnabled());
				m_armchairMaterial->EnableOcclusionMap(!m_armchairMaterial->IsOcclusionMapEnabled());
				m_floorMaterial->EnableOcclusionMap(!m_floorMaterial->IsOcclusionMapEnabled());
				m_floor2Material->EnableOcclusionMap(!m_floor2Material->IsOcclusionMapEnabled());
				break;
			case FIREFLY_KEY_6:
				m_pistolMaterial->EnableHeightMap(!m_pistolMaterial->IsHeightMapEnabled());
				m_globeMaterial->EnableHeightMap(!m_globeMaterial->IsHeightMapEnabled());
				m_armchairMaterial->EnableHeightMap(!m_armchairMaterial->IsHeightMapEnabled());
				m_floorMaterial->EnableHeightMap(!m_floorMaterial->IsHeightMapEnabled());
				m_floor2Material->EnableHeightMap(!m_floor2Material->IsHeightMapEnabled());
				break;
			case FIREFLY_KEY_UP:
				m_heightScale += 0.01f;
				m_floorMaterial->SetHeightScale(m_heightScale);
				m_floor2Material->SetHeightScale(m_heightScale);
				break;
			case FIREFLY_KEY_DOWN:
				m_heightScale = std::max(m_heightScale - 0.01f, 0.f);
				m_floorMaterial->SetHeightScale(m_heightScale);
				m_floor2Material->SetHeightScale(m_heightScale);
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

private:
	std::shared_ptr<Firefly::Renderer> m_renderer;
	std::shared_ptr<Firefly::Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;

	std::shared_ptr<Firefly::Model> m_pistolModel;
	std::shared_ptr<Firefly::Material> m_pistolMaterial;

	std::shared_ptr<Firefly::Model> m_globeModel;
	std::shared_ptr<Firefly::Material> m_globeMaterial;

	std::shared_ptr<Firefly::Model> m_armchairModel;
	std::shared_ptr<Firefly::Material> m_armchairMaterial;

	std::shared_ptr<Firefly::Model> m_floorModel;
	std::shared_ptr<Firefly::Material> m_floorMaterial;

	std::shared_ptr<Firefly::Model> m_floor2Model;
	std::shared_ptr<Firefly::Material> m_floor2Material;
	float m_heightScale = 0.2f;
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}