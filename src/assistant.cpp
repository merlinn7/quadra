#include "assistant.h"
#include "quadra_ui.h"
#include <iostream>
#include <thread>

void quadrasoftware::AiAssistant()
{
	std::thread t([&]()
		{
			ai_assistant->Run();
		});
	t.detach();

	Sleep(1000);
	std::cout << "[AI] Waiting your command..." << std::endl;
}