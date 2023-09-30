#include "planner.hpp"

#include "sensors/GPS.hpp"

Point Planner::waypoints[256];
Point Planner::home;
unsigned short Planner::current_waypoint = 0;
unsigned short Planner::num_waypoints = 0;

bool Planner::resetWaypoint() {
  unsigned short old = Planner::num_waypoints;
  Planner::num_waypoints = 0;
  Planner::current_waypoint = 0;
  return old != 0;
}

void Planner::resetWaypointNum() { Planner::current_waypoint = 0; }

bool Planner::addWaypoint(Point wp) {
  if (Planner::num_waypoints > 255) return false;
  Planner::waypoints[Planner::num_waypoints++] = wp;
  return true;
}

bool Planner::addWaypoint(double lat, double lng) {
  if (Planner::num_waypoints > 255) return false;
  Planner::waypoints[Planner::num_waypoints].setLat(lat);
  Planner::waypoints[Planner::num_waypoints].setLng(lng);
  Planner::num_waypoints++;
  return true;
}

Point Planner::getCurrentWaypoint() { return Planner::waypoints[Planner::current_waypoint]; }
Point Planner::getNextWaypoint() {
  if (Planner::current_waypoint + 1 < Planner::num_waypoints)
    return Planner::waypoints[Planner::current_waypoint + 1];
  else {
    Point p;
    p.setLat(0);
    p.setLng(0);
    return p;
  }
}
double Planner::distanceToCurWaypoint() {
  Point wp = Planner::getCurrentWaypoint();
  return GPS::gps.distanceBetween(GPS::gps.location.lat(), GPS::gps.location.lng(), wp.lat(), wp.lng());
}
unsigned short Planner::nextWaypoint() { return ++Planner::current_waypoint; }

unsigned short Planner::numWaypoints() { return Planner::num_waypoints; }
float Planner::headingToTarget() {
  Point wp = Planner::getCurrentWaypoint();
  return GPS::gps.courseTo(GPS::gps.location.lat(), GPS::gps.location.lng(), wp.lat(), wp.lng());
}

void Planner::setHomePos(Point _home) { Planner::home = _home; }
void Planner::setHomePos() {
  Point _home;
  _home.setLat(GPS::gps.location.lat());
  _home.setLat(GPS::gps.location.lng());
  Planner::setHomePos(_home);
}

Point Planner::getHome() { return Planner::home; }