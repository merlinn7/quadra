#pragma once

#include <mavsdk/mavsdk.h>
using namespace mavsdk;

class QuadraMavInterface
{
public:
	QuadraMavInterface()
	{
		// mavsdk configuration
		Mavsdk::Configuration config = Mavsdk::Configuration{ Mavsdk::ComponentType::GroundStation };
		config.set_always_send_heartbeats(true);
		mavsdk.set_configuration(config);
	}

	Mavsdk mavsdk{ Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation} };
	std::optional<std::shared_ptr<System>> system;
	Mavsdk::ConnectionHandle handle;

	bool Connect(std::string url);
	bool Disconnect();
	bool IsConnected();

} QuadraInterface;