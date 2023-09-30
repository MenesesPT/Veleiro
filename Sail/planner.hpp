#ifndef PLANNER_H
#define PLANNER_H

#define WAYPOINTRADIUS 5

#include "sensors/GPS.hpp"

class Planner {
 private:
  static Point waypoints[256];
  static Point home;
  static unsigned short current_waypoint;
  static unsigned short num_waypoints;

 public:
  static bool addWaypoint(Point wp);
  static bool addWaypoint(double lat, double lng);
  static void setHomePos();
  static void setHomePos(Point _home);
  static Point getCurrentWaypoint();
  static Point getNextWaypoint();
  static double distanceToCurWaypoint();
  static unsigned short nextWaypoint();
  static unsigned short numWaypoints();
  static float headingToTarget();
  static bool resetWaypoint();
  static Point getHome();
  static void resetWaypointNum();
};

#endif  // PLANNER_H