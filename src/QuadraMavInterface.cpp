#include "QuadraMavInterface.h"
#include <windows.h>
#include "quadra_ui.h"

Telemetry::Position QuadraMavInterface::GetPosition()
{
	if (!IsConnected())
		return Telemetry::Position();

	return position;
}

Telemetry::Position QuadraMavInterface::GetTargetPosition()
{
	if (!IsConnected())
		return Telemetry::Position();

	return targetPosition;
}

Telemetry::EulerAngle QuadraMavInterface::GetAngles()
{
	if (!IsConnected())
		return Telemetry::EulerAngle();

	return angles;
}

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
	// when we subscribe, these values always updates themself
	telemetry->subscribe_armed([&](bool armed_) {
		armed = armed_;
		});

	telemetry->subscribe_landed_state([&](Telemetry::LandedState state) {
		landed_state = state;
		});

	telemetry->subscribe_position([&](Telemetry::Position pos) {
		position = pos;
		});

	telemetry->subscribe_attitude_euler([&](Telemetry::EulerAngle euler) {
		angles = euler;
		});

	telemetry->subscribe_vtol_state([&](Telemetry::VtolState state) {
		vtolState = state;
		});

	// subscribe to mavlink message MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT 
	// for getting current target position
	mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT, [&](const mavlink_message_t& message) {
		mavlink_position_target_global_int_t pos_target;
		mavlink_msg_position_target_global_int_decode(&message, &pos_target);

		// thanks chatgpt
		double target_lat = pos_target.lat_int / 1E7;
		double target_lon = pos_target.lon_int / 1E7;

		targetPosition.latitude_deg = target_lat;
		targetPosition.longitude_deg = target_lon;
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

	//action->goto_location(37.41403149, -121.99623803, position.absolute_altitude_m, NULL);

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

Telemetry::VtolState QuadraMavInterface::GetVtolState()
{
	if (!IsConnected())
		return Telemetry::VtolState();

	return vtolState;
}
