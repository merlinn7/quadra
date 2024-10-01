#pragma once

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/transponder/transponder.h>	
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>	
#include <mavsdk/plugins/param/param.h>

using namespace mavsdk;

class QuadraMavInterface
{
public:
	QuadraMavInterface(std::function<void()> iUserFunc = [](){})
	{
		// mavsdk configuration
		Mavsdk::Configuration config = Mavsdk::Configuration{ Mavsdk::ComponentType::GroundStation };
		config.set_always_send_heartbeats(true);
		mavsdk.set_configuration(config);

		userFunc = iUserFunc;
	}

private:
	Mavsdk mavsdk{ Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation} };
	std::optional<std::shared_ptr<System>> system;
	Mavsdk::ConnectionHandle handle;

	// allows user to able to run their own operations
	std::function<void()> userFunc;

	std::unique_ptr<mavsdk::Action> action;
	std::unique_ptr<mavsdk::Telemetry> telemetry;
	std::unique_ptr<mavsdk::Param> param;
	std::unique_ptr<mavsdk::Offboard> offboard;
	std::unique_ptr<mavsdk::MavlinkPassthrough> mavlink_passthrough;

public:
	Telemetry::Position GetPosition();
	Telemetry::EulerAngle GetAngles();

	bool Connect(std::string url);
	bool Disconnect();
	bool IsConnected();
	bool IsArmed();
	bool Arm();
	bool Disarm();
	bool Takeoff(int meters);
	bool Land();

private:
	bool armed = false;
	Telemetry::LandedState landed_state;
	Telemetry::Position position;
	Telemetry::EulerAngle angles;
};