# Multi Map Navigation

A ROS package for multi-map navigation with wormhole transitions for TurtleBot3, implemented in Gazebo.

## Overview
- **Action Server**: Handles navigation goals across multiple maps (`room1`, `room2`) using `MultiMapNavigation.action`.
- **Wormhole Mechanism**: Transitions between maps using `wormholes.db` for seamless multi-map navigation.
- **Trajectory Visualization**: Plots robot trajectory in real-time using `visualize_trajectory.py` with Matplotlib.
- **Dependencies**: ROS Noetic, TurtleBot3, Gazebo, SQLite3, Matplotlib, PyQt5.

## Setup
```bash
# Install ROS Noetic (if not already installed)
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
sudo apt update
sudo apt install ros-noetic-desktop-full
sudo rosdep init
rosdep update
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc

# Install dependencies
sudo apt install ros-noetic-turtlebot3 ros-noetic-turtlebot3-gazebo ros-noetic-map-server ros-noetic-amcl ros-noetic-move-base sqlite3 python3-pip git
pip3 install matplotlib pyqt5

# Create and setup Catkin workspace
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws
catkin_make
echo "source ~/catkin_ws/devel/setup.bash" >> ~/.bashrc
source ~/.bashrc

# Clone the repository
cd ~/catkin_ws/src
git clone https://github.com/yourusername/multi_map_navigation.git

# Build the project
cd ~/catkin_ws
catkin_make
source devel/setup.bash

# Verify setup
rospack find multi_map_navigation
sqlite3 ~/catkin_ws/src/multi_map_navigation/database/wormholes.db "SELECT * FROM wormholes;"
```
Expected `wormholes.db` output:
```
room1|2.0|0.0|0.0|room2|0.0|0.0|0.0
room2|0.0|0.0|0.0|room1|2.0|0.0|0.0
```

## Usage
```bash
# Launch Gazebo
export TURTLEBOT3_MODEL=waffle
roslaunch turtlebot3_gazebo turtlebot3_house.launch

# Launch Navigation (new terminal)
cd ~/catkin_ws
source devel/setup.bash
roslaunch multi_map_navigation multi_map_navigation.launch

# Visualize Trajectory (new terminal)
rosrun multi_map_navigation visualize_trajectory.py

# Send Goal within room1 (new terminal)
rostopic pub -1 /multi_map_navigation_node/multi_map_navigation/goal multi_map_navigation/MultiMapNavigationActionGoal "{header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: ''}, goal_id: {stamp: {secs: $(date +%s), nsecs: 0}, id: 'goal1'}, goal: {goal_pose: {header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: 'map'}, pose: {position: {x: 1.0, y: 1.0, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}}, target_map: 'room1'}}"

# Send Goal from room1 to room2 (new terminal)
rostopic pub -1 /multi_map_navigation_node/multi_map_navigation/goal multi_map_navigation/MultiMapNavigationActionGoal "{header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: ''}, goal_id: {stamp: {secs: $(date +%s), nsecs: 0}, id: 'goal2'}, goal: {goal_pose: {header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: 'map'}, pose: {position: {x: 0.5, y: 0.5, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}}, target_map: 'room2'}}"

# Monitor topics (new terminals)
rostopic echo /multi_map_navigation_node/multi_map_navigation/feedback
rostopic echo /trajectory
rostopic echo /multi_map_navigation_node/multi_map_navigation/result
rostopic echo /move_base/goal

# Launch RViz (new terminal)
rosrun rviz rviz -d ~/catkin_ws/src/multi_map_navigation/config/rviz_config.rviz
```

## File Structure
```
multi_map_navigation/
├── action/
│   └── MultiMapNavigation.action
├── config/
│   └── rviz_config.rviz
├── database/
│   └── wormholes.db
├── include/
│   └── multi_map_navigation/
│       └── MultiMapNavigator.hpp
├── launch/
│   └── multi_map_navigation.launch
├── maps/
│   ├── room1.yaml
│   ├── room1.pgm
│   ├── room2.yaml
│   └── room2.pgm
├── scripts/
│   └── visualize_trajectory.py
├── src/
│   ├── multi_map_navigation_node.cpp
│   └── multi_map_navigator.cpp
├── CMakeLists.txt
├── package.xml
├── README.md
└── demo.bag (optional)
```

## Troubleshooting
```bash
# Matplotlib window not opening
pip install --upgrade matplotlib pyqt5
echo "backend : Qt5Agg" > ~/.matplotlib/matplotlibrc
export DISPLAY=:0
python ~/catkin_ws/test_plot.py

# Goal not processed
cat /home/vinay/.ros/log/latest/*multi_map_navigation_node*.log
rostopic pub -1 /move_base_simple/goal geometry_msgs/PoseStamped "{header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: 'map'}, pose: {position: {x: 0.5, y: 0.5, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}}"

# AMCL issues
rostopic pub -1 /initialpose geometry_msgs/PoseWithCovarianceStamped "{header: {seq: 0, stamp: {secs: $(date +%s), nsecs: 0}, frame_id: 'map'}, pose: {pose: {position: {x: 0.0, y: 0.0, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}, covariance: [0.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.06853892326654787]}}"
```

## Demo
```bash
# Record demo
rosrun rosbag record /scan /tf /trajectory /multi_map_navigation_node/multi_map_navigation/feedback /multi_map_navigation_node/multi_map_navigation/result

# Play demo (if demo.bag included)
rosbag play demo.bag
```

## Submission
- **Repository**: [https://github.com/yourusername/multi_map_navigation](https://github.com/yourusername/multi_map_navigation)
- **Zip (if required)**:
  ```bash
  cd ~/catkin_ws/src
  zip -r multi_map_navigation.zip multi_map_navigation
  ```

---
*Author*: Vinay  
*Date*: May 2025