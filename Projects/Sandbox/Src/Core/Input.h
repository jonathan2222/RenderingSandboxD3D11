#pragma once

#include "Core/Key.h"
#include "Utils/Maths.h"

struct GLFWwindow;
namespace RS
{
	class Input
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(Input);

		static std::shared_ptr<Input> Get();

		void Init();
		void Update();

		bool IsKeyPressed(const Key & key) const;
		bool IsKeyReleased(const Key & key) const;
		KeyState GetKeyState(const Key & key);

		glm::vec2 GetCursorDelta();
		glm::vec2 GetScrollDelta();
		glm::vec2 GetMousePos() const;
		bool IsMBPressed(const MB & button) const;
		bool IsMBReleased(const MB & button) const;

		void CenterMouse() const;
		void LockMouse() const;
		void UnlockMouse() const;

		static std::string KeyStateToStr(KeyState state);

	private:
		static void KeyCallback(GLFWwindow * wnd, int key, int scancode, int action, int mods);
		static void CursorPositionCallback(GLFWwindow * window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
		static void MouseScrollCallback(GLFWwindow * window, double xOffset, double yOffset);

	private:
		inline static std::unordered_map<Key, KeyState> s_KeyMap = std::unordered_map<Key, KeyState>();
		inline static std::unordered_map<MB, KeyState> s_MBMap = std::unordered_map<MB, KeyState>();
		inline static glm::vec2 s_MousePos		= glm::vec2(0.f);
		inline static glm::vec2 s_MousePosPre	= glm::vec2(0.f);
		inline static glm::vec2 s_MouseDelta	= glm::vec2(0.f);
		inline static glm::vec2 s_ScrollDelta	= glm::vec2(0.f);
	};
}