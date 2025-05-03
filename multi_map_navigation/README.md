# Multi-Map Navigation for TurtleBot3

This ROS package implements a multi-map navigation system for TurtleBot3, allowing seamless transitions between rooms using a "wormhole" mechanism. The system uses an action server, SQLite database, and TurtleBot3’s navigation stack.

## Features
- Separate maps for each room with defined wormhole regions.
- SQLite database to store wormhole positions.
- C++ action server for multi-map navigation goals.
- Modular design with OOP principles.
- Trajectory visualization using matplotlib.
- Comprehensive documentation and adherence to ROS C++ style guidelines.

## Prerequisites
- Ubuntu 20.04
- ROS Noetic
- TurtleBot3 packages
- SQLite3
- Python 3 with matplotlib

## Setup
See `Setup Instructions` artifact for detailed installation steps.

## Usage
1. **Launch Gazebo and Navigation**:
   ```bash
   export TURTLEBOT3_MODEL=waffle
   roslaunch turtlebot3_gazebo turtlebot3_house.launch
   roslaunch multi_map_navigation multi_map_navigation.launch
