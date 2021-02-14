#pragma once

#include <GLFW/glfw3.h>

namespace RS
{
	struct DisplayDescription
	{
		std::string		Title = "Untitled Window";
		uint32_t		Width = 1920;
		uint32_t		Height = 1080;
		bool			Fullscreen = false;
		bool			VSync = true;
	};

	class Display
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(Display);

		static std::shared_ptr<Display> Get();

		void Init(const DisplayDescription& description);
		void Release();

		bool ShouldClose() const;
		void Close() noexcept;

		void PollEvents();

		GLFWwindow* GetGLFWWindow();
		HWND GetHWND();

		void SetDescription(const DisplayDescription& description);
		DisplayDescription& GetDescription();

		uint32	GetWidth() const;
		uint32	GetHeight() const;
		float	GetAspectRatio() const;

	private:
		static void ErrorCallback(int error, const char* description);
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

	private:
		inline static Display* m_pSelf = nullptr;

		DisplayDescription	m_Description;
		bool				m_ShouldClose	= false;
		GLFWwindow*			m_pWindow		= nullptr;
		HWND				m_HWND			= nullptr;
	};
}