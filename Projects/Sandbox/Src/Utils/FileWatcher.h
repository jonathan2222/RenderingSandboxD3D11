#pragma once

#include <filesystem>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace RS
{
	class FileWatcher
	{
	public:
		struct FileCallback
		{
			std::function<void(std::vector<std::string>)> callback;
		};

	public:
		FileWatcher();
		~FileWatcher();

		void AddFile(const std::string& filePath);

		void Init(uint32_t delay);
		void Release();

		void AddCallBack(FileCallback callback);

	private:
		void Watcher();

	private:
		std::mutex															m_Mutex;
		bool																m_ShouldRelease = false;
		bool																m_RequestStop = false;
		std::thread* m_pWatcherThread = nullptr;
		std::vector<FileCallback>											m_CallbackList;
		std::unordered_map<std::string, std::filesystem::file_time_type>	m_WatchList;
		std::unordered_map<std::string, std::filesystem::file_time_type>	m_WatchListThread;
		std::vector<std::string>											m_ModifiedFiles;
		std::chrono::duration<uint32_t, std::milli>							m_DelayDuration;
	};
}