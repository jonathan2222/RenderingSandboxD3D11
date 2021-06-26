#include "PreCompiled.h"
#include "FileWatcher.h"

using namespace RS;

FileWatcher::FileWatcher() : m_DelayDuration(std::chrono::milliseconds(1000))
{
}

FileWatcher::~FileWatcher()
{
	if(!m_RequestStop)
		Release();
}

void FileWatcher::AddFile(const std::string& filePath)
{
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		std::filesystem::file_time_type fileTime = std::filesystem::last_write_time(std::filesystem::path(filePath));
		m_WatchList[filePath] = fileTime;
	}
}

void FileWatcher::Init(uint32_t delay)
{
	m_ShouldRelease = true;
	m_RequestStop = false;
	m_DelayDuration = std::chrono::milliseconds(delay);
	m_pWatcherThread = new std::thread(&FileWatcher::Watcher, this);
}

void FileWatcher::Release()
{
	if (m_ShouldRelease)
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_RequestStop = true;
		}

		m_pWatcherThread->join();
		m_ShouldRelease = false;
	}

	delete m_pWatcherThread;
	m_pWatcherThread = nullptr;
}

void FileWatcher::AddCallBack(FileCallback callback)
{
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_CallbackList.push_back(callback);
	}
}

void FileWatcher::Watcher()
{
	bool shouldStop = false;
	while (!shouldStop)
	{
		std::this_thread::sleep_for(m_DelayDuration);

		for (auto& [filePath, type] : m_WatchListThread)
		{
			// Check if the file was modified, if that was the case, add it to the modified list.
			auto path = std::filesystem::path(filePath);
			auto currentType = std::filesystem::last_write_time(path);
			if (type != currentType)
			{
				type = currentType;
				m_ModifiedFiles.push_back(filePath);
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (m_RequestStop)
				shouldStop = true;

			// Add new files to watch.
			for (auto& [filePath, type] : m_WatchList)
			{
				if (m_WatchListThread.contains(filePath) == false)
				{
					m_WatchListThread[filePath] = type;
				}
			}

			// Call all callers, clear the list and start over.
			std::for_each(m_CallbackList.begin(), m_CallbackList.end(),
				[&](auto fCallBack) {
					fCallBack.callback(m_ModifiedFiles);
				}
			);

			m_ModifiedFiles.clear();
		}
	}

	LOG_INFO("Thread Watcher has closed.");
}
