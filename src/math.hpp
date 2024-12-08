#include <iostream>
#include <cmath>
#include <qmath.h>

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

// using this in angle-dependent calculations
// set i to 0 if not using for formation flight pos calculation
std::pair<double, double> FormationPosCalculation(
    double latitude_deg,
    double longitude_deg,
    double yaw_radians,
    double base_distance,
    double spread_distance,
    int i = 0
) {
    double distance_back = base_distance * (i / 2 + 1);
    double lateral_offset = spread_distance * (i / 2 + 1);
    int direction = i % 2 == 0 ? 1 : -1;

    double dx = -distance_back * cos(yaw_radians);
    double dy = -distance_back * sin(yaw_radians);

    double lateral_dx = lateral_offset * cos(yaw_radians + M_PI / 2) * direction;
    double lateral_dy = lateral_offset * sin(yaw_radians + M_PI / 2) * direction;

    double new_lat = latitude_deg + dx;
    double new_lon = longitude_deg + dy;

    new_lat += lateral_dx;
    new_lon += lateral_dy;

    return { new_lat, new_lon };
}

double CalcYaw(double lat1, double lon1, double lat2, double lon2)
{
    lat1 = toRadians(lat1);
    lon1 = toRadians(lon1);
    lat2 = toRadians(lat2);
    lon2 = toRadians(lon2);

    double deltaLon = lon2 - lon1;
    double x = sin(deltaLon) * cos(lat2);
    double y = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(deltaLon);
    double yawRad = atan2(x, y);

    double yawDeg = yawRad * 180.0 / M_PI;

    if (yawDeg < 0) {
        yawDeg += 360.0;
    }

    return yawDeg;
}