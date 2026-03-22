#ifndef GAME_HPP
#define GAME_HPP

#include "audio/audio_engine.h"
#include "input/input_events.h"
#include "pages/page.h"

class Application
{
	public:
		Application();

		void Initialize();
		void Run();
		void Terminate();
		[[nodiscard]] bool IsRunning() const;

	private:
		bool mIsRunning;

		std::shared_ptr<IPage> currentPage;
		std::vector<InputEvent> inputEventsCurrent;

		void Start() const;
		void ProcessEvents();
		static void Update();
		void Render();

		void ChangePage(const std::string_view& pageName);
};
#endif
