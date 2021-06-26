#pragma once

#include "Scenes/Camera.h"
#include "Core/Input.h"
#include "Utils/Timer.h"
#include <glm/gtx/rotate_vector.hpp>

namespace RS
{
	struct CameraUtils
	{
		/*
		*	Camera
		*	Type: Orbit
		*	Controlls:
		*		Pan:			Hold L_SHIFT and drag with the LMB.
		*		Zoom:			Hold L_CTL and drag with the LBM up and down.
		*		Orbit			Drag with the LMB.
		*		Reset target:	Hold the 'C' key.
		* @param camera: The camera to use.
		*/
		static void UpdateOrbitCamera(Camera& camera, const glm::vec3& startingTarget = glm::vec3(0.f, 0.5f, 0.f), bool toTarget = false)
		{
			auto input = Input::Get();
			glm::vec2 delta = input->GetCursorDelta();

			static const glm::vec3 s_StartingTarget = startingTarget;
			static glm::vec3 s_Target = s_StartingTarget;
			uint32 s_CameraState = 0;
			if (input->IsKeyPressed(Key::LEFT_CONTROL))		s_CameraState = 1;
			else if (input->IsKeyPressed(Key::LEFT_SHIFT))	s_CameraState = 2;
			else											s_CameraState = 0;

			Input::Get()->WasKeyClickedOrHeld(Key::C,
				[&]() { // Clicked
				},
				[&]() { // Held
					toTarget = true;
				}, 0.5f, Input::ModFlag::IGNORED);

			if (toTarget)
			{
				s_Target = s_StartingTarget;
				glm::vec3 pos = s_StartingTarget + glm::vec3(0.f, 0.f, 2.0f);
				camera.LookAt(pos, s_Target);
				toTarget = false;
			}

			if (input->IsMBPressed(MB::LEFT))
			{
				float mouseSensitivity = 0.005f;
				float zoomFactor = 2.f;

				glm::vec3 pos = camera.GetPos();
				if (s_CameraState == 0) // Orbit
				{
					// Rotate the distance vector to the target around the y axis and the camera's right direction to get the orbit effect.
					glm::vec3 v = pos - s_Target;
					v = glm::rotate(v, -delta.x * mouseSensitivity, glm::vec3(0.f, 1.f, 0.f));
					v = glm::rotate(v, -delta.y * mouseSensitivity, camera.GetRight());
					pos = s_Target + v;
				}
				else if (s_CameraState == 1) // Zoom
				{
					float zoom = delta.y * mouseSensitivity * zoomFactor;
					glm::vec3 dir = glm::normalize(s_Target - pos);

					// Only zoom if the not too close.
					glm::vec3 dirNext = glm::normalize(s_Target - pos - dir * zoom);
					// Check the polarity of this dir to the next dir. 
					// If the result is negative, the direction was flipped and the zoom should not be applied.
					if (glm::dot(dir, dirNext) > 0.f)
						pos += dir * zoom;
				}
				else if (s_CameraState == 2) // Pan
				{
					// Move the target and the camera one the camera plane (its right and up plane).
					glm::vec3 right = camera.GetRight();
					glm::vec3 up = camera.GetUp();
					glm::vec3 offset = right * -delta.x + up * delta.y;
					offset *= mouseSensitivity;
					pos += offset;
					s_Target += offset;
				}
				camera.LookAt(pos, s_Target);

				// Update the projection for the possibility that the screen size has been changed.
				camera.UpdateProj();
			}
		};

		/*
		*	Camera
		*	Type: FPS
		*	Controlls:
		*		Faster:					Hold L_SHIFT to get a speed boost.
		*		Look around:			Use LMB.
		*		Move:					Use LMB.
		*		Reset target:			Hold the 'C' key.
		*		Enable/Disable Camera:	Press the 'C' key.
		* @param camera: The camera to use.
		*/
		static void UpdateFPSCamera(float dt, Camera& camera)
		{
			auto input = Input::Get();
			glm::vec2 delta = input->GetCursorDelta();

			float speed = 1.f;
			static float s_MouseSensitivity = 0.005f;
			static float s_BoostFactor = 2.f;

			static const glm::vec3 s_StartingDir = camera.GetDir();
			static const glm::vec3 s_StartingPos = camera.GetPos();

			if (input->IsKeyPressed(Key::LEFT_SHIFT))
				speed *= s_BoostFactor;

			static bool s_IsActive = false;
			Input::Get()->WasKeyClickedOrHeld(Key::C,
				[&]() { // Clicked
					s_IsActive = !s_IsActive;

					if (s_IsActive)
						input->LockMouse();
					else
						input->UnlockMouse();
				},
				[&]() { // Held
					camera.LookAt(s_StartingPos, s_StartingPos + s_StartingDir);
				}, 0.5f, Input::ModFlag::IGNORED);

			if (s_IsActive)
			{
				glm::vec3 dir = camera.GetDir();
				glm::vec3 right = camera.GetRight();
				glm::vec3 pos = camera.GetPos();
				if (input->IsKeyPressed(Key::W)) pos += dir * speed * dt;
				if (input->IsKeyPressed(Key::S)) pos -= dir * speed * dt;
				if (input->IsKeyPressed(Key::D)) pos += right * speed * dt;
				if (input->IsKeyPressed(Key::A)) pos -= right * speed * dt;

				dir = glm::rotate(dir, -delta.x * s_MouseSensitivity, glm::vec3(0.f, 1.f, 0.f));
				dir = glm::rotate(dir, -delta.y * s_MouseSensitivity, right);

				camera.LookAt(pos, pos + dir);

				// Update the projection for the possibility that the screen size has been changed.
				camera.UpdateProj();
			}
		};
	};
}