#pragma once

namespace RS
{
	class Scene
	{
	public:
		Scene(const std::string& name);
		virtual ~Scene() = default;

		virtual void Start() = 0;

		virtual void Selected() = 0;
		virtual void Unselected() = 0;

		virtual void End() = 0;

		virtual void FixedTick() = 0;

		virtual void Tick(float dt) = 0;

		void SetName(const std::string& name);
		std::string GetName();

	private:
		uint32_t GenerateIndex() const;

	private:
		std::string m_Name;
	};
}