# Multi Map Navigation

A ROS package for multi-map navigation with wormhole transitions for TurtleBot3, implemented in Gazebo.

## Overview
- **Action Server**: Handles navigation goals across multiple maps (`room1`, `room2`).
- **Wormhole Mechanism**: Transitions between maps using `wormholes.db`.
- **Trajectory Visualization**: Plots robot trajectory using `visualize_trajectory.py`.
- **Dependencies**: ROS Noetic, TurtleBot3, Gazebo, SQLite3, Matplotlib.

## Setup
```bash
cd ~/catkin_ws/src
git clone https://github.com/yourusername/multi_map_navigation.git
cd ~/catkin_ws
catkin_make
source devel/setup.bash
