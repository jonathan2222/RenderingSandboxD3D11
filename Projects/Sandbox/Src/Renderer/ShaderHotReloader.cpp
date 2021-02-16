#include "PreCompiled.h"
#include "ShaderHotReloader.h"

using namespace RS;

FileWatcher								ShaderHotReloader::s_fileWatcher;
std::vector<std::pair<Shader*, bool>>	ShaderHotReloader::s_Shaders;
std::mutex								ShaderHotReloader::s_Mutex;
bool									ShaderHotReloader::s_ShouldUpdate;

void ShaderHotReloader::Init()
{
	// Check for update each second
	s_fileWatcher.Init(1000);

	// Add a callback function which checks for shaders which should be reloaded.
	FileWatcher::FileCallback callback;
	callback.callback = [](std::vector<std::string> files)
	{
		std::lock_guard<std::mutex> lock(s_Mutex);
		s_ShouldUpdate = true;

		for (auto& shader : s_Shaders)
		{
			auto& shaderFiles = shader.first->GetFiles();
			for (auto shaderFile : shaderFiles)
			{
				if (std::count(files.begin(), files.end(), shaderFile))
					shader.second = true;
			}
		}
	};
	s_fileWatcher.AddCallBack(callback);
}

void ShaderHotReloader::Release()
{
	s_fileWatcher.Release();
}

void ShaderHotReloader::AddShader(Shader* shader)
{
	// Add a shader to the watcher.
	if (shader != nullptr)
	{
		auto& files = shader->GetFiles();
		for (auto& file : files)
		{
			s_fileWatcher.AddFile(file);
		}

		s_Shaders.push_back(std::make_pair(shader, false));
	}
	else
	{
		LOG_WARNING("The shader pointer cannot be a nullptr");
	}
}

void ShaderHotReloader::Update()
{
	// If shaders sould be updated, reload them.
	std::lock_guard<std::mutex> lock(s_Mutex);
	if (s_ShouldUpdate)
	{
		for (auto& shader : s_Shaders)
		{
			if (shader.second)
			{
				shader.second = false;
				shader.first->Reload();
			}
		}

		s_ShouldUpdate = false;
	}
}
