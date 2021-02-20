#pragma once

#include "Utils/Maths.h"

namespace RS
{
	class Camera
	{
	public:
		RS_DEFAULT_CLASS(Camera);

		void Init(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float nearPlane, float farPlane, float fov);

		float GetNearPlane() const;
		float GetFarPlane() const;

		glm::vec3 GetPos() const;
		glm::vec3 GetDir() const;

		void UpdateView();
		void UpdateProj();

		glm::vec3 GetUp() const;
		glm::vec3 GetRight() const;

		void SetOrientaion(glm::vec3 up, glm::vec3& right);
		void SetOrientaion(float yaw, float pitch);

		void LookAt(const glm::vec3& target);
		void LookAt(const glm::vec3& pos, const glm::vec3& target);

		void SetPosition(glm::vec3 pos);

		glm::mat4 GetView() const;
		glm::mat4 GetProj() const;

	private:
		glm::vec3 m_Pos;
		glm::vec3 m_Dir;
		glm::vec3 m_Up;
		float m_NearPlane;
		float m_FarPlane;
		float m_FOV;

		glm::mat4 m_View;
		glm::mat4 m_Proj;
	};
}