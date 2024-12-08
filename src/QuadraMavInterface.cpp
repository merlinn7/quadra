#include "QuadraMavInterface.h"
#include <windows.h>
#include "quadra_ui.h"
#include <iostream>

Telemetry::Position QuadraMavInterface::GetPosition()
{
	Telemetry::Position null_pos;
	null_pos.absolute_altitude_m = 0;
	null_pos.relative_altitude_m = 0;
	null_pos.latitude_deg = 0;
	null_pos.longitude_deg = 0;

	if (!IsConnected())
		return null_pos;

	if (isnan(position.latitude_deg) || isnan(position.longitude_deg) || isnan(position.absolute_altitude_m) || isnan(position.relative_altitude_m))
		return null_pos;

	return position;
}

Telemetry::Position QuadraMavInterface::GetHome()
{
	Telemetry::Position null_pos;
	null_pos.absolute_altitude_m = 0;
	null_pos.relative_altitude_m = 0;
	null_pos.latitude_deg = 0;
	null_pos.longitude_deg = 0;

	if (!IsConnected())
		return null_pos;

	if (isnan(home.latitude_deg) || isnan(home.longitude_deg) || isnan(home.absolute_altitude_m) || isnan(home.relative_altitude_m))
		return null_pos;

	return home;
}

Telemetry::Position QuadraMavInterface::GetTargetPosition()
{
	Telemetry::Position null_pos;
	null_pos.absolute_altitude_m = 0;
	null_pos.relative_altitude_m = 0;
	null_pos.latitude_deg = 0;
	null_pos.longitude_deg = 0;

	if (!IsConnected())
		return null_pos;

	return targetPosition;
}

Telemetry::EulerAngle QuadraMavInterface::GetAngles()
{
	Telemetry::EulerAngle null_angles;
	null_angles.pitch_deg = 0;
	null_angles.roll_deg = 0;
	null_angles.yaw_deg = 0;
	null_angles.timestamp_us = 0;

	if (!IsConnected())
		return null_angles;

	return angles;
}

int QuadraMavInterface::GetSystemId()
{
	if (!IsConnected())
		return -1;

	return system->get()->get_system_id();
}

class test_vtol_state
{
public:
	Telemetry::VtolState state = Telemetry::VtolState::Undefined;
};

bool QuadraMavInterface::Connect(std::string url)
{
	// send connection request
	auto connectionResult = mavsdk.add_any_connection_with_handle(url);
	if (connectionResult.first == ConnectionResult::Success)
		system = mavsdk.first_autopilot(3.0);

	// check connection success
	if (!system.has_value() || !system.value().get() || !system.value().get()->is_connected())
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
	mission_raw = std::make_unique<mavsdk::MissionRaw>(system.value());

	// subscribe for values we need. 
	// when we subscribe, these values always updates themself
	telemetry->subscribe_armed([&](bool armed_) {
		armed = armed_;
		});

	telemetry->subscribe_landed_state([&](Telemetry::LandedState state) {
		landed_state = state;
		});

	telemetry->subscribe_position([&](Telemetry::Position pos) {
		if (!isnan(pos.latitude_deg) && !isnan(pos.longitude_deg) && !isnan(pos.absolute_altitude_m) && !isnan(pos.relative_altitude_m))
			position = pos;
		});

	telemetry->subscribe_attitude_euler([&](Telemetry::EulerAngle euler) {
		if (!isnan(euler.pitch_deg) && !isnan(euler.roll_deg) && !isnan(euler.yaw_deg) && !isnan(euler.timestamp_us))
			angles = euler;
		});

	telemetry->subscribe_flight_mode([&](Telemetry::FlightMode mode) {
		flightMode = mode;
		});

	telemetry->subscribe_gps_info([&](Telemetry::GpsInfo info) {
		gpsInfo = info;
		});

	telemetry->subscribe_landed_state([&](Telemetry::LandedState state) {
		landedState = state;
		});

	telemetry->subscribe_health_all_ok([&](bool Health) {
		health = Health;
		});

	telemetry->subscribe_velocity_ned([&](Telemetry::VelocityNed ned) {
		velocityNed = ned;
		});

	telemetry->subscribe_fixedwing_metrics([&](Telemetry::FixedwingMetrics metrics) {
		fixedwingMetrics = metrics;
		});

	telemetry->subscribe_battery([&](Telemetry::Battery i_battery) {
		battery = i_battery;
		});

	telemetry->subscribe_home([&](Telemetry::Position i_home) {
		home = i_home;
		});

	test_vtol_state* old_vtol_state = new test_vtol_state;
	telemetry->subscribe_vtol_state([this, old_vtol_state](Telemetry::VtolState state) {
		if (old_vtol_state->state != state)
		{
			old_vtol_state->state = state;
			bool value = true;
			memcpy(&refresh_map, &value, 1);
		}

		vtolState = state;
		});

	mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_SERVO_OUTPUT_RAW, [&](const mavlink_message_t& message) {
		mavlink_servo_output_raw_t servo_output_raw;
		mavlink_msg_servo_output_raw_decode(&message, &servo_output_raw);

		actuators.actuator1_rpm = servo_output_raw.servo1_raw;
		actuators.actuator2_rpm = servo_output_raw.servo2_raw;
		actuators.actuator3_rpm = servo_output_raw.servo3_raw;
		actuators.actuator4_rpm = servo_output_raw.servo4_raw;
		actuators.actuator5_rpm = servo_output_raw.servo5_raw;
		});

	// subscribe to mavlink message MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT 
	// for getting current target position
	mavlink_passthrough->subscribe_message(MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT, [&](const mavlink_message_t& message) {
		mavlink_position_target_global_int_t pos_target;
		mavlink_msg_position_target_global_int_decode(&message, &pos_target);

		// thanks chatgpt
		double target_lat = pos_target.lat_int / 1E7;
		double target_lon = pos_target.lon_int / 1E7;

		if (!isnan(target_lat) && !isnan(target_lon))
		{
			targetPosition.latitude_deg = target_lat;
			targetPosition.longitude_deg = target_lon;
		}
		});

	// save handle for disconnection
	handle = connectionResult.second;
	connectionUrl = url;
	return true;
}

bool QuadraMavInterface::Disconnect()
{
	if (!IsConnected())
		return false;

	mavsdk.remove_connection(handle);
	system.reset();

	return true;
}

bool QuadraMavInterface::DisconnectForce()
{
	mavsdk.remove_connection(handle);
	system.reset();

	return true;
}

bool QuadraMavInterface::IsConnected()
{
	// check system object is valid and connected
	if (this && system && system.has_value() && system.value() && system.value().get() && system.value().get()->is_connected())
		return true;
	else
		return false;
}

bool QuadraMavInterface::Shutdown()
{
	if (!IsConnected())
		return false;

	return action->shutdown() == Action::Result::Success;
}

bool QuadraMavInterface::Reboot()
{
	if (!IsConnected())
		return false;

	return action->reboot() == Action::Result::Success;
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
	NEW_CMD();

	if (!IsConnected() || !IsArmed())
		return false;

	auto result = action->set_takeoff_altitude(meters);
	if (result != Action::Result::Success)
		return false;

	result = action->takeoff();
	if (result != Action::Result::Success)
		return false;

	bool flag = false;

	// basic check for if takeoff complete via flight mode
	while (true)
	{
		CHECK_CMD();

		if (flightMode == Telemetry::FlightMode::Takeoff)
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

		userFunc();
	}

	return true;
}

bool QuadraMavInterface::Land()
{
	NEW_CMD();

	if (!IsConnected())
		return false;

	auto result = action->land();
	if (result != Action::Result::Success)
		return false;

	bool flag = false;

	// basic check for if landing complete via landed_state
	while (true)
	{
		CHECK_CMD();

		if (landedState == Telemetry::LandedState::Landing)
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

		userFunc();
	}

	return true;
}

bool QuadraMavInterface::TransitionToDrone()
{
	if (!IsConnected())
		return false;

	auto result = action->transition_to_multicopter();
	if (result != Action::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::TransitionToFixedwing()
{
	if (!IsConnected())
		return false;

	auto result = action->transition_to_fixedwing();
	if (result != Action::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::GoToLocation(double latitude, double longitude, float altitude, double proximityLimit, float yaw)
{
	NEW_CMD();
	
	if (!IsConnected() || !IsArmed())
		return false;

	auto result = action->goto_location(latitude, longitude, altitude, yaw);
	if (result != Action::Result::Success)
		return false;

	while (fabs(position.latitude_deg - latitude) > proximityLimit || fabs(position.longitude_deg - longitude) > proximityLimit) 
	{
		CHECK_CMD();
		userFunc();
	}

	return true;
}

bool QuadraMavInterface::Hold()
{
	if (!IsConnected())
		return false;

	return action->hold() == Action::Result::Success;
}

bool QuadraMavInterface::RTL()
{
	if (!IsConnected())
		return false;

	return action->return_to_launch() == Action::Result::Success;
}

Telemetry::VtolState QuadraMavInterface::GetVtolState()
{
	if (!IsConnected())
		return Telemetry::VtolState();

	return vtolState;
}

Telemetry::FlightMode QuadraMavInterface::GetFlightMode()
{
	if (!IsConnected())
		return Telemetry::FlightMode::Unknown;

	return flightMode;
}

Telemetry::GpsInfo QuadraMavInterface::GetGPSInfo()
{
	if (!IsConnected())
		return Telemetry::GpsInfo{ 0, Telemetry::FixType::NoGps };;

	return gpsInfo;
}

Telemetry::LandedState QuadraMavInterface::GetLandedState()
{
	if (!IsConnected())
		return Telemetry::LandedState::Unknown;

	return landedState;
}

Telemetry::VelocityNed QuadraMavInterface::GetVelocityNed()
{
	if (!IsConnected())
		return Telemetry::VelocityNed();

	return velocityNed;
}

Telemetry::FixedwingMetrics QuadraMavInterface::GetFixedwingMetrics()
{
	if (!IsConnected())
		return Telemetry::FixedwingMetrics();

	return fixedwingMetrics;
}

Telemetry::Battery QuadraMavInterface::GetBattery()
{
	if (!IsConnected())
		return Telemetry::Battery();

	return battery;
}

Param::AllParams QuadraMavInterface::GetAllParams()
{
	if (!IsConnected())
		return Param::AllParams();

	return param->get_all_params();
}

bool QuadraMavInterface::SetParamFloat(std::string param, float value)
{
	if (!IsConnected())
		return false;

	auto result = this->param->set_param_float(param, value);
	if (result != Param::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::SetParamInt(std::string param, int value)
{
	if (!IsConnected())
		return false;

	auto result = this->param->set_param_int(param, value);
	if (result != Param::Result::Success)
		return false;

	return true;
}

Actuators QuadraMavInterface::GetActuators()
{
	if (!IsConnected())
		return Actuators();

	return actuators;
}

bool QuadraMavInterface::GetHealth()
{
	if (!IsConnected())
		return false;

	return health;
}

std::string QuadraMavInterface::GetConnectionUrl()
{
	return connectionUrl;
}

std::pair<bool, std::vector<MissionRaw::MissionItem>> QuadraMavInterface::DownloadMission()
{
	std::pair<bool, std::vector<MissionRaw::MissionItem>> null_value{false, std::vector<MissionRaw::MissionItem>()};

	if (!IsConnected())
		return null_value;

	auto result = mission_raw->download_mission();
	if (result.first != MissionRaw::Result::Success)
	{
		return null_value;
	}

	return std::pair<bool, std::vector<MissionRaw::MissionItem>>(true, result.second);
}

bool QuadraMavInterface::StartMission()
{
	if (!IsConnected())
		return false;

	auto result = mission_raw->start_mission();
	if (result != MissionRaw::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::ClearMission()
{
	if (!IsConnected())
		return false;

	auto result = mission_raw->clear_mission();
	if (result != MissionRaw::Result::Success)
		return false;

	return true;
}

bool QuadraMavInterface::UploadMission(std::vector<MissionRaw::MissionItem> mission)
{
	if (!IsConnected())
		return false;

	auto result = mission_raw->upload_mission(mission);
	if (result != MissionRaw::Result::Success)
		return false;

	return true;
}

using namespace std;
void QuadraMavInterface::test()
{
	action->set_current_speed(20.0f);
}

int QuadraMavInterface::NewCommand()
{
	currentCmd++;
	return currentCmd;
}

bool QuadraMavInterface::CheckCommand(int cmd)
{
	return (cmd == currentCmd && IsConnected());
}