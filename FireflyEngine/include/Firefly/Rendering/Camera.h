#pragma once

#include <glm/glm.hpp>

namespace Firefly
{
	class Camera
	{
	public:
		enum class ProjectionMode
		{
			PERSPECTIVE,
			ORTHOGRAPHIC
		};

		Camera(int width, int height);
		~Camera();

		void LookAt(const glm::vec3& target);

		void SetPosition(const glm::vec3& position);
		void SetHeight(int height);
		void SetWidth(int width);
		void SetNearPlane(float nearPLane);
		void SetFarPlane(float farPlane);
		void SetFieldOfView(float fieldOfView);
		void SetProjectionMode(ProjectionMode projectionMode);

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix() const;
		glm::vec3 GetPosition() const;
		glm::vec3 GetViewDirection() const;
		glm::vec3 GetRightDirection() const;
		int GetHeight() const;
		int GetWidth() const;
		float GetAspectRatio() const;
		float GetNearPlane() const;
		float GetFarPlane() const;
		float GetFieldOfView() const;
		ProjectionMode GetProjectionMode() const;

	private:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projectionMatrix;
		ProjectionMode m_projectionMode = ProjectionMode::PERSPECTIVE;

		glm::vec3 m_position;
		glm::vec3 m_viewDirection;
		glm::vec3 m_rightDirection;
		static const glm::vec3 s_worldUpDirection;

		float m_fieldOfView;
		int m_height;
		int m_width;
		float m_aspectRatio;
		float m_nearPlane;
		float m_farPlane;
	};
}