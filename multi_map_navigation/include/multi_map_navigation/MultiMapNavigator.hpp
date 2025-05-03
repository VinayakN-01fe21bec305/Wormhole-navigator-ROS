#ifndef MULTI_MAP_NAVIGATOR_HPP
#define MULTI_MAP_NAVIGATOR_HPP

#include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <geometry_msgs/PoseStamped.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <multi_map_navigation/MultiMapNavigationAction.h> // Add this include

namespace multi_map_navigation {

struct Wormhole {
    std::string source_map;
    double source_x, source_y, source_yaw;
    std::string target_map;
    double target_x, target_y, target_yaw;
};

class MultiMapNavigator {
public:
    MultiMapNavigator();
    ~MultiMapNavigator();

private:
    void executeCallback(const multi_map_navigation::MultiMapNavigationGoalConstPtr &goal);
    bool loadWormholes();
    bool switchMap(const std::string &map_name);
    void publishInitialPose(const geometry_msgs::PoseStamped &pose);
    void logTrajectory(const geometry_msgs::PoseStamped &pose);

    ros::NodeHandle nh_;
    actionlib::SimpleActionServer<multi_map_navigation::MultiMapNavigationAction> as_;
    actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> move_base_client_;
    ros::Publisher initial_pose_pub_;
    ros::Publisher trajectory_pub_;
    sqlite3 *db_;
    std::string current_map_;
    std::vector<Wormhole> wormholes_;
    std::string db_path_;
    std::string maps_dir_;
};

} // namespace multi_map_navigation

#endif // MULTI_MAP_NAVIGATOR_HPP
