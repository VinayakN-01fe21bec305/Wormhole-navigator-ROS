#include "multi_map_navigation/MultiMapNavigator.hpp"

int main(int argc, char **argv) {
    ros::init(argc, argv, "multi_map_navigation_node");
    multi_map_navigation::MultiMapNavigator navigator;
    ros::spin();
    return 0;
}
