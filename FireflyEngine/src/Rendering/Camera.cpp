#include "pch.h"
#include "Rendering/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	const glm::vec3 Camera::s_worldUpDirection = glm::vec3(0.f, 1.f, 0.f);

	Camera::Camera(int width, int height) :
		m_width(width),
		m_height(height),
		m_aspectRatio((float)m_width / (float)m_height),
		m_fieldOfView(45.f),
		m_nearPlane(0.1f),
		m_farPlane(100.f),
		m_position(glm::vec3(0.f, 0.f, 0.f)),
		m_viewDirection(glm::vec3(0.f, 0.f, -1.f)),
		m_rightDirection(glm::vec3(1.f, 0.f, 0.f)),
		m_projectionMode(Camera::ProjectionMode::PERSPECTIVE)
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	Camera::~Camera()
	{
	}

	void Camera::LookAt(const glm::vec3& target)
	{
		m_viewDirection = target - m_position;
		UpdateViewMatrix();
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_position = position;
		UpdateViewMatrix();
	}
	void Camera::SetHeight(int height)
	{
		m_height = height;
		m_aspectRatio = (float)m_width / (float)m_height;
		UpdateProjectionMatrix();
	}
	void Camera::SetWidth(int width)
	{
		m_width = width;
		m_aspectRatio = (float)m_width / (float)m_height;
		UpdateProjectionMatrix();
	}
	void Camera::SetNearPlane(float nearPLane)
	{
		m_nearPlane = nearPLane;
		UpdateProjectionMatrix();
	}
	void Camera::SetFarPlane(float farPlane)
	{
		m_farPlane = farPlane;
		UpdateProjectionMatrix();
	}
	void Camera::SetFieldOfView(float fieldOfView)
	{
		m_fieldOfView = fieldOfView;
		UpdateProjectionMatrix();
	}
	void Camera::SetProjectionMode(ProjectionMode projectionMode)
	{
		m_projectionMode = projectionMode;
		UpdateProjectionMatrix();
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		return m_viewMatrix;
	}
	glm::mat4 Camera::GetProjectionMatrix() const
	{
		return m_projectionMatrix;
	}
	glm::vec3 Camera::GetPosition() const
	{
		return m_position;
	}
	glm::vec3 Camera::GetViewDirection() const
	{
		return m_viewDirection;
	}
	glm::vec3 Camera::GetRightDirection() const
	{
		return m_rightDirection;
	}
	int Camera::GetHeight() const
	{
		return m_height;
	}
	int Camera::GetWidth() const
	{
		return m_width;
	}
	float Camera::GetAspectRatio() const
	{
		return m_aspectRatio;
	}
	float Camera::GetNearPlane() const
	{
		return m_nearPlane;
	}
	float Camera::GetFarPlane() const
	{
		return m_farPlane;
	}
	float Camera::GetFieldOfView() const
	{
		return m_fieldOfView;
	}
	Camera::ProjectionMode Camera::GetProjectionMode() const
	{
		return m_projectionMode;
	}

	void Camera::UpdateViewMatrix()
	{
		m_viewMatrix = glm::lookAt(m_position, m_position + m_viewDirection, s_worldUpDirection);
		m_rightDirection = glm::normalize(glm::cross(m_viewDirection, s_worldUpDirection));
	}

	void Camera::UpdateProjectionMatrix()
	{
		switch (m_projectionMode)
		{
		case ProjectionMode::ORTHOGRAPHIC:
			m_projectionMatrix = glm::ortho(-(float)m_width / 200.f, (float)m_width / 200.f, -(float)m_height / 200.f, (float)m_height / 200.f, m_nearPlane, m_farPlane);
			break;
		case ProjectionMode::PERSPECTIVE:
		default:
			m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspectRatio, m_nearPlane, m_farPlane);
			break;
		}
	}
}