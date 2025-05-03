#include "multi_map_navigation/MultiMapNavigator.hpp"
#include <std_msgs/String.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <tf/transform_datatypes.h>

namespace multi_map_navigation {

MultiMapNavigator::MultiMapNavigator()
    : nh_("~"),
      as_(nh_, "multi_map_navigation", boost::bind(&MultiMapNavigator::executeCallback, this, _1), false),
      move_base_client_("move_base", true),
      db_(nullptr) {
    // Load parameters
    nh_.param<std::string>("db_path", db_path_, "/home/vinay/catkin_ws/src/multi_map_navigation/database/wormholes.db");
    nh_.param<std::string>("maps_dir", maps_dir_, "/home/vinay/catkin_ws/src/multi_map_navigation/maps/");

    // Initialize publishers
    initial_pose_pub_ = nh_.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    trajectory_pub_ = nh_.advertise<geometry_msgs::PoseStamped>("/trajectory", 10);

    // Initialize database
    if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
        ROS_ERROR("Failed to open database: %s", sqlite3_errmsg(db_));
        return;
    }

    // Load wormholes
    if (!loadWormholes()) {
        ROS_ERROR("Failed to load wormholes from database");
        return;
    }

    // Initialize current map
    current_map_ = "room1"; // Match launch file's initial map
    ROS_INFO("Initialized current_map_ to: %s", current_map_.c_str());

    // Wait for move_base server
    if (!move_base_client_.waitForServer(ros::Duration(10.0))) {
        ROS_ERROR("Failed to connect to move_base action server");
    } else {
        ROS_INFO("Connected to move_base action server");
    }

    // Start action server
    as_.start();
    ROS_INFO("MultiMapNavigator initialized");
}

MultiMapNavigator::~MultiMapNavigator() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool MultiMapNavigator::loadWormholes() {
    const char *sql = "SELECT source_map, source_x, source_y, source_yaw, target_map, target_x, target_y, target_yaw FROM wormholes";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        ROS_ERROR("SQL prepare failed: %s", sqlite3_errmsg(db_));
        return false;
    }

    wormholes_.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Wormhole wh;
        wh.source_map = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        wh.source_x = sqlite3_column_double(stmt, 1);
        wh.source_y = sqlite3_column_double(stmt, 2);
        wh.source_yaw = sqlite3_column_double(stmt, 3);
        wh.target_map = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        wh.target_x = sqlite3_column_double(stmt, 5);
        wh.target_y = sqlite3_column_double(stmt, 6);
        wh.target_yaw = sqlite3_column_double(stmt, 7);
        wormholes_.push_back(wh);
        ROS_INFO("Loaded wormhole: %s -> %s", wh.source_map.c_str(), wh.target_map.c_str());
    }

    sqlite3_finalize(stmt);
    return true;
}

bool MultiMapNavigator::switchMap(const std::string &map_name) {
    // Update map server with new map
    std::string map_file = maps_dir_ + map_name + ".yaml";
    std::string cmd = "rosrun map_server map_server " + map_file + " &";
    if (system(cmd.c_str()) != 0) {
        ROS_ERROR("Failed to load map: %s", map_name.c_str());
        return false;
    }
    current_map_ = map_name;
    ROS_INFO("Switched to map: %s", map_name.c_str());
    return true;
}

void MultiMapNavigator::publishInitialPose(const geometry_msgs::PoseStamped &pose) {
    geometry_msgs::PoseWithCovarianceStamped initial_pose;
    initial_pose.header = pose.header;
    initial_pose.pose.pose = pose.pose;
    initial_pose.pose.covariance[0] = 0.25;
    initial_pose.pose.covariance[7] = 0.25;
    initial_pose.pose.covariance[35] = 0.06853892326654787;
    initial_pose_pub_.publish(initial_pose);
    ROS_INFO("Published initial pose: x=%f, y=%f", pose.pose.position.x, pose.pose.position.y);
}

void MultiMapNavigator::logTrajectory(const geometry_msgs::PoseStamped &pose) {
    trajectory_pub_.publish(pose);
    ROS_INFO("Logged trajectory pose: x=%f, y=%f", pose.pose.position.x, pose.pose.position.y);
}

void MultiMapNavigator::executeCallback(const multi_map_navigation::MultiMapNavigationGoalConstPtr &goal) {
    ROS_INFO("Received goal for target_map: %s, pose: x=%f, y=%f", goal->target_map.c_str(), 
             goal->goal_pose.pose.position.x, goal->goal_pose.pose.position.y);
    multi_map_navigation::MultiMapNavigationFeedback feedback;
    multi_map_navigation::MultiMapNavigationResult result;

    // Log current pose
    feedback.current_map = current_map_;
    feedback.current_pose = goal->goal_pose;
    as_.publishFeedback(feedback);
    logTrajectory(goal->goal_pose);
    ROS_INFO("Published initial feedback and trajectory");

    // Check if goal is in current map
    ROS_INFO("Current map: %s, Target map: %s", current_map_.c_str(), goal->target_map.c_str());
    if (goal->target_map == current_map_) {
        ROS_INFO("Goal is in current map, sending to move_base");
        move_base_msgs::MoveBaseGoal mb_goal;
        mb_goal.target_pose = goal->goal_pose;
        move_base_client_.sendGoal(mb_goal);
        ROS_INFO("Sent goal to move_base: x=%f, y=%f", mb_goal.target_pose.pose.position.x, 
                 mb_goal.target_pose.pose.position.y);
        move_base_client_.waitForResult();

        if (move_base_client_.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
            ROS_INFO("move_base succeeded");
            result.success = true;
            as_.setSucceeded(result);
        } else {
            ROS_ERROR("move_base failed with state: %s", move_base_client_.getState().toString().c_str());
            result.success = false;
            as_.setAborted(result);
        }
        return;
    }

    // Find wormhole to target map
    ROS_INFO("Searching for wormhole from %s to %s", current_map_.c_str(), goal->target_map.c_str());
    for (const auto &wh : wormholes_) {
        if (wh.source_map == current_map_ && wh.target_map == goal->target_map) {
            ROS_INFO("Found wormhole: %s -> %s", wh.source_map.c_str(), wh.target_map.c_str());
            // Navigate to wormhole
            move_base_msgs::MoveBaseGoal wh_goal;
            wh_goal.target_pose.header.frame_id = "map";
            wh_goal.target_pose.header.stamp = ros::Time::now();
            wh_goal.target_pose.pose.position.x = wh.source_x;
            wh_goal.target_pose.pose.position.y = wh.source_y;
            wh_goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(wh.source_yaw);
            move_base_client_.sendGoal(wh_goal);
            ROS_INFO("Sent wormhole goal to move_base: x=%f, y=%f", wh_goal.target_pose.pose.position.x, 
                     wh_goal.target_pose.pose.position.y);
            move_base_client_.waitForResult();

            if (move_base_client_.getState() != actionlib::SimpleClientGoalState::SUCCEEDED) {
                ROS_ERROR("Failed to reach wormhole, move_base state: %s", 
                          move_base_client_.getState().toString().c_str());
                result.success = false;
                as_.setAborted(result);
                return;
            }

            // Switch map
            ROS_INFO("Switching to map: %s", wh.target_map.c_str());
            if (!switchMap(wh.target_map)) {
                ROS_ERROR("Failed to switch map to %s", wh.target_map.c_str());
                result.success = false;
                as_.setAborted(result);
                return;
            }

            // Set initial pose in new map
            geometry_msgs::PoseStamped initial_pose;
            initial_pose.header.frame_id = "map";
            initial_pose.header.stamp = ros::Time::now();
            initial_pose.pose.position.x = wh.target_x;
            initial_pose.pose.position.y = wh.target_y;
            initial_pose.pose.orientation = tf::createQuaternionMsgFromYaw(wh.target_yaw);
            publishInitialPose(initial_pose);
            ROS_INFO("Published initial pose in new map");

            // Navigate to final goal
            move_base_msgs::MoveBaseGoal mb_goal;
            mb_goal.target_pose = goal->goal_pose;
            move_base_client_.sendGoal(mb_goal);
            ROS_INFO("Sent final goal to move_base: x=%f, y=%f", mb_goal.target_pose.pose.position.x, 
                     mb_goal.target_pose.pose.position.y);
            move_base_client_.waitForResult();

            if (move_base_client_.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
                ROS_INFO("move_base succeeded for final goal");
                result.success = true;
                as_.setSucceeded(result);
            } else {
                ROS_ERROR("move_base failed for final goal, state: %s", 
                          move_base_client_.getState().toString().c_str());
                result.success = false;
                as_.setAborted(result);
            }
            return;
        }
    }

    ROS_ERROR("No wormhole found from %s to %s", current_map_.c_str(), goal->target_map.c_str());
    result.success = false;
    as_.setAborted(result);
}

} // namespace multi_map_navigation
