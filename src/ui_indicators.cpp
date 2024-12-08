#include "quadra_ui.h"
#include <format>

double calculate_percentage(double part, double whole) {
    if (whole == 0) {
        return 0.0;
    }
    return (part / whole) * 100.0;
}

void quadrasoftware::HandleIndicators()
{
	// Mode Indicator
	std::string mode_indicator_str = "MODE   : ";
    std::string mode_styleSheet = "color: green;";
    switch (QuadraInterface->GetFlightMode())
    {
    case Telemetry::FlightMode::Ready:
        mode_indicator_str += "READY";
        break;
    case Telemetry::FlightMode::Takeoff:
        mode_indicator_str += "TAKEOFF";
        break;
    case Telemetry::FlightMode::Hold:
        mode_indicator_str += "HOLD";
        break;
    case Telemetry::FlightMode::Mission:
        mode_indicator_str += "MISSION";
        break;
    case Telemetry::FlightMode::ReturnToLaunch:
        mode_indicator_str += "RTL";
        break;
    case Telemetry::FlightMode::Land:
        mode_indicator_str += "LAND";
        break;
    case Telemetry::FlightMode::Offboard:
        mode_indicator_str += "OFFBOARD";
        break;
    case Telemetry::FlightMode::FollowMe:
        mode_indicator_str += "FOLLOW ME";
        break;
    case Telemetry::FlightMode::Manual:
        mode_indicator_str += "MANUAL";
        break;
    case Telemetry::FlightMode::Altctl:
        mode_indicator_str += "ALTCTL";
        break;
    case Telemetry::FlightMode::Posctl:
        mode_indicator_str += "POSCTL";
        break;
    case Telemetry::FlightMode::Acro:
        mode_indicator_str += "ACRO";
        break;
    case Telemetry::FlightMode::Stabilized:
        mode_indicator_str += "STABILIZED";
        break;
    case Telemetry::FlightMode::Rattitude:
        mode_indicator_str += "RATTITUDE";
        break;
    case Telemetry::FlightMode::Unknown:
    default:
        mode_indicator_str += "UNKNOWN";
        mode_styleSheet = "color: red;";
        break;
    }

    ui.modeLabel->setText(QString::fromStdString(mode_indicator_str));
    ui.modeLabel->setStyleSheet(QString::fromStdString(mode_styleSheet));

    // GPS Indicator
    std::string gps_indicator_str = "GPS    : ";
    std::string gps_styleSheet = "color: green;";
    switch (QuadraInterface->GetGPSInfo().fix_type)
    {
    case Telemetry::FixType::NoGps:
        gps_indicator_str += "NO GPS";
        gps_styleSheet = "color: red;";
        break;
    case Telemetry::FixType::NoFix:
        gps_indicator_str += "NO FIX";
        gps_styleSheet = "color: red;";
        break;
    case Telemetry::FixType::Fix2D:
        gps_indicator_str += "FIX 2D";
        break;
    case Telemetry::FixType::Fix3D:
        gps_indicator_str += "FIX 3D";
        break;
    case Telemetry::FixType::FixDgps:
        gps_indicator_str += "FIX DGPS";
        break;
    case Telemetry::FixType::RtkFloat:
        gps_indicator_str += "RTK FLOAT";
        break;
    case Telemetry::FixType::RtkFixed:
        gps_indicator_str += "RTK FIXED";
        break;
    default:
        gps_indicator_str += "UNKNOWN";
        gps_styleSheet = "color: red;";
        break;
    }

    ui.gpsLabel->setText(QString::fromStdString(gps_indicator_str));
    ui.gpsLabel->setStyleSheet(QString::fromStdString(gps_styleSheet));

    // State indicator
    std::string state_indicator_str = "STATE  : ";
    std::string state_styleSheet = "color: green;";
    switch (QuadraInterface->GetLandedState())
    {
    case Telemetry::LandedState::Unknown:
        state_indicator_str += "UNKNOWN";
        state_styleSheet = "color: red;";
        break;
    case Telemetry::LandedState::OnGround:
        state_indicator_str += "ON GROUND";
        break;
    case Telemetry::LandedState::InAir:
        state_indicator_str += "IN AIR";
        break;
    case Telemetry::LandedState::TakingOff:
        state_indicator_str += "TAKING OFF";
        break;
    case Telemetry::LandedState::Landing:
        state_indicator_str += "LANDING";
        break;
    default:
        state_indicator_str += "UNKNOWN";
        state_styleSheet = "color: red;";
        break;
    }

    ui.stateLabel->setText(QString::fromStdString(state_indicator_str));
    ui.stateLabel->setStyleSheet(QString::fromStdString(state_styleSheet));

    // Satallite indicator
    std::string sats_indicator_str = "SATS   : ";
    std::string sats_styleSheet = "color: green;";

    sats_indicator_str += std::to_string(QuadraInterface->GetGPSInfo().num_satellites);
    if (QuadraInterface->GetGPSInfo().num_satellites <= 5)
        sats_styleSheet = "color: red;";
    ui.satsLabel->setText(QString::fromStdString(sats_indicator_str));
    ui.satsLabel->setStyleSheet(QString::fromStdString(sats_styleSheet));

    // Health indicator
    std::string health_indicator_str = "HEALTH : ";
    std::string health_styleSheet = "color: green;";
    switch (QuadraInterface->GetHealth())
    {
    case false:
        health_indicator_str += "UNHEALTHY";
        health_styleSheet = "color: red;";
        break;
    case true:
        health_indicator_str += "OK";
        break;
    default:
        health_indicator_str += "UNHEALTHY";
        health_styleSheet = "color: red;";
        break;
    }

    ui.healthLabel->setText(QString::fromStdString(health_indicator_str));
    ui.healthLabel->setStyleSheet(QString::fromStdString(health_styleSheet));

    std::string altitude_label_str = std::format("{:.1f}m | {:.1f} m/s",
        QuadraInterface->GetPosition().relative_altitude_m,
        -QuadraInterface->GetVelocityNed().down_m_s);
    ui.altitudeLabel->setText(QString::fromStdString(altitude_label_str));

    float horizontal_speed = std::sqrt(
        std::pow(QuadraInterface->GetVelocityNed().north_m_s, 2) +
        std::pow(QuadraInterface->GetVelocityNed().east_m_s, 2));
    std::string velocity_label_str = std::format("{:.1f} m/s", horizontal_speed);
    ui.velocityLabel->setText(QString::fromStdString(velocity_label_str));

    std::string roll_label_str = std::format("{:.1f}", QuadraInterface->GetAngles().roll_deg);
    ui.rollLabel->setText(QString::fromStdString(roll_label_str));

    std::string throttle_label_str = std::format("{:.1f}%", QuadraInterface->GetFixedwingMetrics().throttle_percentage);
    ui.throttleLabel->setText(QString::fromStdString(throttle_label_str));

    std::string location_label_str = std::format("{:.6f}, {:.6f}", QuadraInterface->GetPosition().latitude_deg, QuadraInterface->GetPosition().longitude_deg);
    ui.locationLabel->setText(QString::fromStdString(location_label_str));

    std::string battery_label_str = std::format("{:.1f}%", QuadraInterface->GetBattery().remaining_percent);
    ui.batteryLabel->setText(QString::fromStdString(battery_label_str));

    auto actuators = QuadraInterface->GetActuators();
    progressBar1->setValue(calculate_percentage(actuators.actuator1_rpm, max_rpm));
    progressBar2->setValue(calculate_percentage(actuators.actuator2_rpm, max_rpm));
    progressBar3->setValue(calculate_percentage(actuators.actuator3_rpm, max_rpm));
    progressBar4->setValue(calculate_percentage(actuators.actuator4_rpm, max_rpm));
    progressBar5->setValue(calculate_percentage(actuators.actuator5_rpm, max_rpm));
}