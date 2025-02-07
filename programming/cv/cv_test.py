import cv2
import numpy as np
from realsense_data import DepthCamera


point = [0,0]
def show_distance(event, x, y, args, params):
    global point
    if 0 <= x < depth_frame.shape[1] and 0 <= y < depth_frame.shape[0]:
        point = [x,y]

# Initialize
realsense_D435 = DepthCamera()

cv2.namedWindow("RealSense Color with Depths")
cv2.setMouseCallback("RealSense Color with Depths", show_distance)

while True:

    ret, depth_frame, color_frame = realsense_D435.get_frame()


    depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_frame, alpha=0.03), cv2.COLORMAP_JET)
    full_frame = np.hstack((color_frame, depth_colormap))
    color_frame_mouse = color_frame
    # creat distance overlay
    cv2.circle(color_frame_mouse, point, 4, (0, 0, 255))
    distance = depth_frame[point[1], point[0]]
    #cv2.putText(full_frame, "{}mm".format(distance), (point[0], point[1] - 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 0), 2)
    cv2.putText(color_frame_mouse, "{}mm".format(distance), (point[0], point[1] - 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 0), 2)

    # Show images
    cv2.imshow('RealSense Color and Depth', full_frame)
    cv2.imshow("RealSense Color with Depths", color_frame_mouse)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

realsense_D435.release()
cv2.destroyAllWindows()