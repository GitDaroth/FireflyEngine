#pragma once

#include <Rendering/Camera.h>
#include <Event/MouseEvent.h>
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
	float m_rotationalSpeed;
	float m_zoomSpeed;
	float m_yaw;
	float m_pitch;
	float m_oldMouseXPos;
	float m_oldMouseYPos;
};