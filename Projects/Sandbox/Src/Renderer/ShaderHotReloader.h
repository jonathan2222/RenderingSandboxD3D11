#pragma once

#include "Renderer/Shader.h"
#include "Utils/FileWatcher.h"

#include <mutex>

namespace RS
{
	class ShaderHotReloader
	{
	public:
		static void Init();
		static void Release();

		static void AddShader(Shader* shader);

		static void Update();

	private:
		static FileWatcher								s_fileWatcher;
		static std::vector<std::pair<Shader*, bool>>	s_Shaders;
		static std::mutex								s_Mutex;
		static bool										s_ShouldUpdate;
	};
}