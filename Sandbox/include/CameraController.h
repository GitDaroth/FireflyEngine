#pragma once

#include <Firefly/Rendering/Camera.h>
#include <Firefly/Input/Input.h>
#include <memory.h>

class CameraController
{
public:
	CameraController(std::shared_ptr<Firefly::Camera> camera);
	~CameraController();

	void OnUpdate(float deltaTime);
	void OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event);

private:
	std::shared_ptr<Firefly::Camera> m_camera;
	float m_linearSpeed;
	float m_linearSpeedBoost;
	float m_gamepadAxisSensitivity;
	float m_gamepadTriggerSensitivity;
	float m_mouseMoveSensitivity;
	float m_mouseWheelSensitivity;
	float m_yaw;
	float m_pitch;
	float m_oldMouseXPos;
	float m_oldMouseYPos;
};