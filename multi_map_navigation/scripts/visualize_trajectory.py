#!/usr/bin/env python
import rospy
import matplotlib.pyplot as plt
from geometry_msgs.msg import PoseStamped
from multi_map_navigation.msg import MultiMapNavigationFeedback
import threading
import queue

class TrajectoryVisualizer:
    def __init__(self):
        rospy.loginfo("Initializing TrajectoryVisualizer")
        self.x = []
        self.y = []
        self.current_map = None
        self.data_queue = queue.Queue()
        self.lock = threading.Lock()
        
        # ROS subscribers
        rospy.loginfo("Setting up subscribers")
        self.sub = rospy.Subscriber("/trajectory", PoseStamped, self.trajectory_callback)
        self.feedback_sub = rospy.Subscriber("/multi_map_navigation/feedback", MultiMapNavigationFeedback, self.feedback_callback)
        
        # Matplotlib setup
        rospy.loginfo("Setting up Matplotlib")
        plt.ion()
        self.fig, self.ax = plt.subplots()
        self.line, = self.ax.plot(self.x, self.y, 'b-', label='Trajectory')
        self.ax.set_xlabel('X (m)')
        self.ax.set_ylabel('Y (m)')
        self.ax.set_title('Robot Trajectory')
        self.ax.grid(True)
        self.ax.legend()
        rospy.loginfo("Matplotlib plot initialized")
        
        # Timer for thread-safe plot updates
        self.timer = self.fig.canvas.new_timer(interval=100)
        self.timer.add_callback(self.update_plot)
        self.timer.start()
        rospy.loginfo("Timer started")

    def trajectory_callback(self, msg):
        rospy.loginfo("Received trajectory message")
        try:
            with self.lock:
                self.data_queue.put(('trajectory', (msg.pose.position.x, msg.pose.position.y)))
        except Exception as e:
            rospy.logerr(f"Error in trajectory_callback: {e}")

    def feedback_callback(self, msg):
        rospy.loginfo(f"Received feedback: current_map={msg.current_map}")
        try:
            if self.current_map != msg.current_map:
                rospy.loginfo(f"Map switched to {msg.current_map}. Clearing trajectory.")
                with self.lock:
                    self.current_map = msg.current_map
                    self.data_queue.put(('clear', None))
        except Exception as e:
            rospy.logerr(f"Error in feedback_callback: {e}")

    def update_plot(self):
        try:
            while not self.data_queue.empty():
                with self.lock:
                    item_type, data = self.data_queue.get()
                if item_type == 'clear':
                    self.x = []
                    self.y = []
                    self.line.set_xdata(self.x)
                    self.line.set_ydata(self.y)
                elif item_type == 'trajectory':
                    x, y = data
                    self.x.append(x)
                    self.y.append(y)
                    self.line.set_xdata(self.x)
                    self.line.set_ydata(self.y)
                self.ax.relim()
                self.ax.autoscale_view()
                self.fig.canvas.draw()
                self.fig.canvas.flush_events()
        except Exception as e:
            rospy.logerr(f"Error in update_plot: {e}")

    def shutdown(self):
        rospy.loginfo("Shutting down TrajectoryVisualizer")
        self.timer.stop()
        plt.close(self.fig)

if __name__ == '__main__':
    try:
        rospy.init_node('trajectory_visualizer', anonymous=True)
        rospy.loginfo("TrajectoryVisualizer node initialized")
        vis = TrajectoryVisualizer()
        rospy.on_shutdown(vis.shutdown)
        rospy.spin()
    except rospy.ROSInterruptException:
        rospy.loginfo("Node interrupted")
    except Exception as e:
        rospy.logerr(f"Error in main: {e}")
