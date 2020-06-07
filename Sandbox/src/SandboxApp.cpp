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

		std::shared_ptr<Firefly::Material> pistolMaterial = std::make_shared<Firefly::Material>(shader);
		pistolMaterial->SetAlbedo(pistolAlbedoTexture);
		pistolMaterial->SetNormal(pistolRoughnessTexture);
		pistolMaterial->SetRoughness(pistolRoughnessTexture);
		pistolMaterial->SetMetalness(pistolMetalnessTexture);
		pistolMaterial->SetOcclusion(pistolOcclusionTexture);

		m_pistolModel = std::make_shared<Firefly::Model>(pistolMesh, pistolMaterial);
		m_pistolModel->SetModelMatrix(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));
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

	}

	virtual void OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event) override
	{
		m_cameraController->OnMouseEvent(event);
	}

	std::shared_ptr<Firefly::Renderer> m_renderer;
	std::shared_ptr<Firefly::Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;
	std::shared_ptr<Firefly::Model> m_pistolModel;
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}