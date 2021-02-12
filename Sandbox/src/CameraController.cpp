#include "CameraController.h"

CameraController::CameraController(std::shared_ptr<Firefly::Camera> camera) :
    m_camera(camera),
    m_linearSpeed(2.5f),
    m_linearSpeedBoost(4.f),
    m_gamepadAxisSensitivity(100.f),
    m_gamepadTriggerSensitivity(20.f),
    m_mouseMoveSensitivity(0.1f),
    m_mouseWheelSensitivity(2.f),
    m_yaw(-90.f),
    m_pitch(0.f),
    m_oldMouseXPos(Firefly::Input::GetMousePositionX()),
    m_oldMouseYPos(Firefly::Input::GetMousePositionY())
{
}

CameraController::~CameraController()
{
}

void CameraController::OnUpdate(float deltaTime)
{
    glm::vec3 camPosition = m_camera->GetPosition();

    float velocity = m_linearSpeed * deltaTime;

    if (Firefly::Input::IsGamepadConnected(FIREFLY_GAMEPAD_1))
    {
        if (Firefly::Input::IsGamepadButtonPressed(FIREFLY_GAMEPAD_1, FIREFLY_GAMEPAD_BUTTON_A))
            velocity *= m_linearSpeedBoost;

        camPosition += Firefly::Input::GetGamepadAxisLeftX(FIREFLY_GAMEPAD_1) * m_camera->GetRightDirection() * velocity;
        camPosition -= Firefly::Input::GetGamepadAxisLeftY(FIREFLY_GAMEPAD_1) * m_camera->GetViewDirection() * velocity;
        m_camera->SetPosition(camPosition);


        m_yaw += Firefly::Input::GetGamepadAxisRightX(FIREFLY_GAMEPAD_1) * m_gamepadAxisSensitivity * deltaTime;
        m_pitch -= Firefly::Input::GetGamepadAxisRightY(FIREFLY_GAMEPAD_1) * m_gamepadAxisSensitivity * deltaTime;
        m_pitch = std::max(std::min(m_pitch, 89.f), -89.f);

        glm::vec3 camViewDirection;
        camViewDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        camViewDirection.y = sin(glm::radians(m_pitch));
        camViewDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        camViewDirection = glm::normalize(camViewDirection);

        m_camera->LookAt(m_camera->GetPosition() + camViewDirection);


        float zoom = m_camera->GetFieldOfView();
        zoom -= (Firefly::Input::GetGamepadTriggerRight(FIREFLY_GAMEPAD_1) + 1.f) * 0.5f * m_gamepadTriggerSensitivity * deltaTime;
        zoom += (Firefly::Input::GetGamepadTriggerLeft(FIREFLY_GAMEPAD_1) + 1.f) * 0.5f * m_gamepadTriggerSensitivity * deltaTime;
        zoom = std::max(std::min(zoom, 45.f), 1.f);
        m_camera->SetFieldOfView(zoom);
    }
    else
    {
        if (Firefly::Input::IsKeyPressed(FIREFLY_KEY_LEFT_SHIFT))
            velocity *= m_linearSpeedBoost;

        if (Firefly::Input::IsKeyPressed(FIREFLY_KEY_W))
            camPosition += m_camera->GetViewDirection() * velocity;
        if (Firefly::Input::IsKeyPressed(FIREFLY_KEY_S))
            camPosition -= m_camera->GetViewDirection() * velocity;
        if (Firefly::Input::IsKeyPressed(FIREFLY_KEY_A))
            camPosition -= m_camera->GetRightDirection() * velocity;
        if (Firefly::Input::IsKeyPressed(FIREFLY_KEY_D))
            camPosition += m_camera->GetRightDirection() * velocity;

        m_camera->SetPosition(camPosition);
    }
}

void CameraController::OnMouseEvent(std::shared_ptr<Firefly::MouseEvent> event)
{
    if (Firefly::Input::IsGamepadConnected(FIREFLY_GAMEPAD_1))
        return;

    if (auto moveEvent = event->AsType<Firefly::MouseMoveEvent>())
    {
        float newMouseXPos = (float)moveEvent->GetXPos();
        float newMouseYPos = (float)moveEvent->GetYPos();
        float deltaX = m_oldMouseXPos - newMouseXPos;
        float deltaY = m_oldMouseYPos - newMouseYPos;
        m_oldMouseXPos = newMouseXPos;
        m_oldMouseYPos = newMouseYPos;

        if (Firefly::Input::IsMouseButtonPressed(FIREFLY_MOUSE_BUTTON_RIGHT))
        {
            m_yaw -= deltaX * m_mouseMoveSensitivity;
            m_pitch += deltaY * m_mouseMoveSensitivity;
            m_pitch = std::max(std::min(m_pitch, 89.f), -89.f);

            glm::vec3 camViewDirection;
            camViewDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            camViewDirection.y = sin(glm::radians(m_pitch));
            camViewDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            camViewDirection = glm::normalize(camViewDirection);

            m_camera->LookAt(m_camera->GetPosition() + camViewDirection);
        }
    }
    else if (auto scrollEvent = event->AsType<Firefly::MouseScrollEvent>())
    {
        float delta = scrollEvent->GetYOffset();
        float zoom = m_camera->GetFieldOfView();

        zoom -= delta * m_mouseWheelSensitivity;
        zoom = std::max(std::min(zoom, 45.f), 1.f);

        m_camera->SetFieldOfView(zoom);
    }
}