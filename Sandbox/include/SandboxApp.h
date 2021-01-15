#pragma once

#include <Firefly/FireflyEngine.h>

#include "CameraController.h"

class SandboxApp : public Firefly::Application
{
public:
	SandboxApp();
	~SandboxApp();

protected:
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnWindowEvent(std::shared_ptr<Firefly::WindowEvent> event) override;
	virtual void OnKeyEvent(std::shared_ptr<Firefly::KeyEvent> event) override;
	virtual void OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event) override;
	virtual void OnGamepadEvent(std::shared_ptr<Firefly::GamepadEvent> event) override;

private:
	std::shared_ptr<Firefly::Scene> m_scene;
	std::shared_ptr<Firefly::Renderer> m_renderer;
	std::shared_ptr<Firefly::Camera> m_camera;
	std::shared_ptr<CameraController> m_cameraController;
	float m_heightScale = 0.2f;
};