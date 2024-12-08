#pragma once

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/transponder/transponder.h>	
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>	
#include <mavsdk/plugins/param/param.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>

using namespace mavsdk;

class Actuators
{
public:
	uint16_t actuator1_rpm;
	uint16_t actuator2_rpm;
	uint16_t actuator3_rpm;
	uint16_t actuator4_rpm;
	uint16_t actuator5_rpm;
};

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
	Mavsdk mavsdk{ Mavsdk::Configuration{ Mavsdk::ComponentType::GroundStation } };
	std::optional<std::shared_ptr<System>> system;
	std::string connectionUrl;
	Mavsdk::ConnectionHandle handle;

	// allows user to able to run their own operations
	std::function<void()> userFunc;

	std::unique_ptr<mavsdk::Action> action;
	std::unique_ptr<mavsdk::Telemetry> telemetry;
	std::unique_ptr<mavsdk::Param> param;
	std::unique_ptr<mavsdk::Offboard> offboard;
	std::unique_ptr<mavsdk::MavlinkPassthrough> mavlink_passthrough;
	std::unique_ptr<mavsdk::MissionRaw> mission_raw;

public:
	Param::AllParams GetAllParams();
	bool SetParamCustom(std::string param, std::string value);
	bool SetParamFloat(std::string param, float value);
	bool SetParamInt(std::string param, int value);

	Telemetry::Position GetPosition();
	Telemetry::Position GetHome();
	Telemetry::Position GetTargetPosition();
	Telemetry::EulerAngle GetAngles();
	Telemetry::VtolState GetVtolState();
	Telemetry::FlightMode GetFlightMode();
	Telemetry::GpsInfo GetGPSInfo();
	Telemetry::LandedState GetLandedState();
	Telemetry::VelocityNed GetVelocityNed();
	Telemetry::FixedwingMetrics GetFixedwingMetrics();
	Telemetry::Battery GetBattery();
	Actuators GetActuators();
	bool GetHealth();
	std::string GetConnectionUrl();

	std::pair<bool, std::vector<MissionRaw::MissionItem>> DownloadMission();
	bool StartMission();
	bool ClearMission();
	bool UploadMission(std::vector<MissionRaw::MissionItem> mission);

	void test();

	int GetSystemId();
	bool Connect(std::string url);
	bool Disconnect();
	bool DisconnectForce();
	bool IsConnected();
	bool Shutdown();
	bool Reboot();
	bool IsArmed();
	bool Arm();
	bool Disarm();
	bool Takeoff(int meters);
	bool Land();
	bool TransitionToDrone();
	bool TransitionToFixedwing();
	bool GoToLocation(double latitude, double longitude, float altitude, double proximityLimit = 0.0010000, float yaw = NAN);
	bool Hold();
	bool RTL();

private:
	bool armed = false;
	Telemetry::LandedState landed_state;
	Telemetry::EulerAngle angles;
	Telemetry::VtolState vtolState;
	Telemetry::Position home;
	Telemetry::Position position;
	Telemetry::Position targetPosition;
	Telemetry::FlightMode flightMode;
	Telemetry::GpsInfo gpsInfo;
	Telemetry::LandedState landedState;
	Telemetry::VelocityNed velocityNed;
	Telemetry::FixedwingMetrics fixedwingMetrics;
	Telemetry::Battery battery;
	Actuators actuators;
	bool health;

	int NewCommand();
	bool CheckCommand(int cmd);
	int currentCmd = 0;
};

// for simple command synchronization
// (not perfect but improving performance at least)
#define NEW_CMD() int thisCmd = NewCommand()
#define CHECK_CMD() if (!CheckCommand(thisCmd)) { break; }