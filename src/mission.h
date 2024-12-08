#pragma once
#include "quadra_ui.h"

class DefineMissionItem
{
public:
	DefineMissionItem(int mavlink_mission, std::string quadra_mission) : mavlink_mission(mavlink_mission), quadra_mission(quadra_mission) {}
	int mavlink_mission;
	std::string quadra_mission;
};
std::vector<DefineMissionItem> defined_mission_items{
	DefineMissionItem(MAV_CMD_NAV_VTOL_TAKEOFF, "VTOL_TAKEOFF"),
	DefineMissionItem(MAV_CMD_NAV_WAYPOINT, "WAYPOINT"),
	DefineMissionItem(MAV_CMD_NAV_LAND, "LAND"),
	DefineMissionItem(MAV_CMD_DO_CHANGE_SPEED, "CHANGE_SPEED"),
	DefineMissionItem(MAV_CMD_NAV_TAKEOFF, "TAKEOFF")
};