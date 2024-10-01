#include "QuadraMavInterface.h"
#include <windows.h>

bool QuadraMavInterface::Connect(std::string url)
{
	// send connection request
	auto connectionResult = mavsdk.add_any_connection_with_handle(url);
	system = mavsdk.first_autopilot(3.0);

	// check connection success
	if (connectionResult.first != mavsdk::ConnectionResult::Success ||
		!system || !system.value().get() || !system.value().get()->is_connected())
	{
		system.reset();
		return false;
	}

	// initialize mavsdk objects
	action = std::make_unique<mavsdk::Action>(system.value());
	telemetry = std::make_unique<mavsdk::Telemetry>(system.value());
	param = std::make_unique<mavsdk::Param>(system.value());
	offboard = std::make_unique<mavsdk::Offboard>(system.value());
	mavlink_passthrough = std::make_unique<mavsdk::MavlinkPassthrough>(system.value());

	// subscribe for values we need. 
	// when we subscribe, these values always updates itself
	telemetry->subscribe_armed([&](bool armed_) {
		armed = armed_;
		});

	telemetry->subscribe_landed_state([&](Telemetry::LandedState state) {
		landed_state = state;
		});

	// save handle for disconnection
	handle = connectionResult.second;
	return true;
}

bool QuadraMavInterface::Disconnect()
{
	if (!IsConnected())
		return false;

	mavsdk.remove_connection(handle);
	system.reset(); // fix: reset system object otherwise crashes
	return true;
}

bool QuadraMavInterface::IsConnected()
{
	// check system object is valid and connected
	if (system && system.value() && system.value().get()->is_connected())
		return true;
	else
		return false;
}

bool QuadraMavInterface::IsArmed()
{
	if (!IsConnected())
		return false;

	return armed;
}

bool QuadraMavInterface::Arm()
{
	if (!IsConnected())
		return false;

	auto result = action->arm();
	if (result != Action::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::Disarm()
{
	if (!IsConnected())
		return false;

	auto result = action->disarm();
	if (result != Action::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::Takeoff(int meters)
{
	if (!IsConnected() || !IsArmed())
		return false;

	auto result = action->set_takeoff_altitude(meters);
	if (result != Action::Result::Success)
		return false;

	result = action->takeoff();
	if (result != Action::Result::Success)
		return false;

	auto state = telemetry->landed_state();
	bool flag = false;

	// basic check for if takeoff complete via landed_state object
	while (true)
	{
		if (state == Telemetry::LandedState::TakingOff)
		{
			flag = true;
		}
		else
		{
			if (flag)
			{
				break;
			}
		}

		state = telemetry->landed_state();
		userFunc();
	}

	return true;
}

bool QuadraMavInterface::Land()
{
	if (!IsConnected())
		return false;

	auto result = action->land();
	if (result != Action::Result::Success)
		return false;

	auto state = telemetry->landed_state();
	bool flag = false;

	// basic check for if landing complete via landed_state object
	while (true)
	{
		if (state == Telemetry::LandedState::Landing)
		{
			flag = true;
		}
		else
		{
			if (flag)
			{
				break;
			}
		}

		state = telemetry->landed_state();
		userFunc();
	}

	return true;
}
