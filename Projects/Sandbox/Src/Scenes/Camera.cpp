#include "PreCompiled.h"
#include "Camera.h"

#include "Core/Display.h"

#include <glm/gtx/rotate_vector.hpp>

using namespace RS;

void Camera::Init(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float nearPlane, float farPlane, float fov)
{
    m_Pos = pos;
    m_Dir = dir;
    m_Up = up;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_FOV = fov;
    UpdateView();
    UpdateProj();
}

float Camera::GetNearPlane() const
{
    return m_NearPlane;
}

float Camera::GetFarPlane() const
{
    return m_FarPlane;
}

glm::vec3 Camera::GetPos() const
{
    return m_Pos;
}

glm::vec3 Camera::GetDir() const
{
    return m_Dir;
}

void Camera::UpdateView()
{
    m_View = glm::lookAtRH(m_Pos, m_Pos + m_Dir, m_Up);
}

void Camera::UpdateProj()
{
    float aspectRatio = Display::Get()->GetAspectRatio();
    m_Proj = glm::perspectiveRH(m_FOV, aspectRatio, m_NearPlane, m_FarPlane);
}

glm::vec3 Camera::GetUp() const
{
    return m_Up;
}

glm::vec3 Camera::GetRight() const
{
    return glm::normalize(glm::cross(m_Dir, m_Up));
}

void Camera::SetOrientaion(glm::vec3 up, glm::vec3& right)
{
    m_Up = glm::normalize(up);
    m_Dir = glm::normalize(glm::cross(up, right));
    UpdateView();
}

void Camera::SetOrientaion(float yaw, float pitch)
{
    glm::vec3 up = GetUp();
    glm::vec3 right = GetRight();
    const glm::vec3 globalUp(0.0f, 1.0f, 0.0f);
    right = glm::rotate(right, yaw, globalUp);
    if (glm::length(up - globalUp) > 0.0001f)
        up = glm::rotate(up, yaw, globalUp);
    SetOrientaion(up, right);

    up = GetUp();
    right = GetRight();
    up = glm::rotate(up, pitch, right);
    SetOrientaion(up, right);
}

void Camera::LookAt(const glm::vec3& target)
{
    LookAt(m_Pos, target);
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& target)
{
    m_Pos = pos;
    m_Dir = glm::normalize(target - pos);
    m_View = glm::lookAtRH(pos, target, m_Up);
}

void Camera::SetPosition(glm::vec3 pos)
{
    m_Pos = pos;
    UpdateView();
}

glm::mat4 Camera::GetView() const
{
    return m_View;
}

glm::mat4 Camera::GetProj() const
{
    return m_Proj;
}
