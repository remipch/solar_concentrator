import cv2
import numpy as np
from panel_web_access import *
import math

red = (0,0,204)
cyan = (204,204,0)
yellow = (0,255,255)
green = (0,204,0)
gray = (128,128,128)

# tolerance arround target
target_tol_px = 20

# defined by Ctrl+Clic on image
target_pos = None

# threshold to detect start blobs and finish blobs from diff image
removed_hot_blob_max_level = 45
new_hot_blob_min_level = 220

# dictionary to map img name to image
debug_images = {}

# dictionary associating direction to the most probable move angle obtained
# initialized by predefined value
# update after each real move
direction_angles = {}
angle = math.pi/3
for direction in MotorsDirection:
    direction_angles[direction] = angle
    angle = angle+math.pi/3

tracking_enabled = False

# replace all fixed-time wait by infinite wait for debug purpose
# press a key to unlock
wait_at_each_step = True

# print and flush
def prn(text):
    print(text,flush=True)

# draw a point in all debug images
def draw_point(x,y,color):
    for img_name, img in debug_images.items():
        img[int(y),int(x)] = color
        cv2.imshow(img_name, img)

# draw a line in all debug images
def draw_line(x0,y0,x1,y1,color):
    for img_name, img in debug_images.items():
        cv2.line(img,(x0,y0),(x1,y1),color)
        cv2.imshow(img_name, img)

def draw_target():
    if target_pos is None:
        return
    target_x = target_pos[0]
    target_y = target_pos[1]
    if tracking_enabled:
        color = green
    else:
        color = gray
    draw_line(target_x-target_tol_px,target_y,target_x+target_tol_px,target_y,color)
    draw_line(target_x,target_y-target_tol_px,target_x,target_y+target_tol_px,color)

def set_target_and_start_tracking(target_x,target_y):
    global tracking_enabled,target_pos
    target_pos = (target_x,target_y)
    tracking_enabled = True
    draw_target()

def on_clic(event,x,y,flags,img_name):
    global target_pos

    if event == cv2.EVENT_LBUTTONDOWN and flags & cv2.EVENT_FLAG_CTRLKEY:
        prn(f"Ctrl+Clic image {img_name} at {x},{y} (new target defined)")
        set_target_and_start_tracking(x,y)

def add_debug_image(img_name,img):
    prn(f"add debug image {img_name}")
    debug_img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    cv2.imshow(img_name, debug_img)
    cv2.setMouseCallback(img_name, on_clic, img_name)
    debug_images[img_name] = debug_img
    return debug_img

def add_image(img_name,img):
    prn(f"add image {img_name}")
    cv2.imshow(img_name, img)
    cv2.setMouseCallback(img_name, on_clic, img_name)
    debug_images[img_name] = img
    return img

def load(img_path):
    img = cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
    #img = cv2.divide(img, 2)
    prn(f"{img_path} mean: {cv2.mean(img)}")
    return img

def compute_blob_centroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise Exception("Cannot compute blob centroid")
    x = int(M["m10"] / M["m00"])
    y = int(M["m01"] / M["m00"])
    return x,y

def compute_spot_center(previous_img,current_img):
    diff_img = (128+cv2.divide(current_img, 2))-cv2.divide(previous_img, 2)
    diff_norm_img = cv2.normalize(diff_img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
    add_debug_image("diff_norm",diff_norm_img)

    removed_hot_blob_img = cv2.threshold(diff_norm_img, removed_hot_blob_max_level, 255, cv2.THRESH_TOZERO_INV)[1]
    new_hot_blob_img = cv2.threshold(diff_norm_img, new_hot_blob_min_level, 255, cv2.THRESH_TOZERO)[1]
    add_debug_image("blobs",removed_hot_blob_img+new_hot_blob_img)

    removed_blob_x,removed_blob_y = compute_blob_centroid(removed_hot_blob_img)
    new_blob_x,new_blob_y = compute_blob_centroid(new_hot_blob_img)

    spot_center_x = (removed_blob_x+new_blob_x)/2
    spot_center_y = (removed_blob_y+new_blob_y)/2
    prn(f"spot_center: {spot_center_x},{spot_center_y}")

    draw_point(removed_blob_x,removed_blob_y,cyan)
    draw_point(new_blob_x,new_blob_y,red)
    draw_point(spot_center_x,spot_center_y,yellow)

    return (spot_center_x,spot_center_y)

# update opencv ui
# listen keyboard and do corresponding actions
# return True if a keyboard event has been triggered and managed
# in this case the caller must abort its current work and return to the main loop
def update_ui(milliseconds):
    if milliseconds==0:
        print("wait key forever")
    key = cv2.waitKey(milliseconds)
    if key==27:
        # exit on escape key
        exit(0)
    prn(f"key: {key}")
    # todo :
    # 't' : tracking
    # 's' : stop tracking
    # 1,4,7,3,6,9 : move on step
    # 0 : stop motors
    return False

def compare_angles(a, b):
    diff = math.fmod(a - b, 2*math.pi)
    if diff > math.pi:
        diff -= 2*math.pi
    elif diff <= -math.pi:
        diff += 2*math.pi
    return diff

def update_motors_direction_history(direction,previous_center,spot_center):
    angle_rad = math.atan2(spot_center[1]-previous_center[1],spot_center[0]-previous_center[0])
    prn(f"update_motors_direction_history: {direction} -> {angle_rad} rad")
    direction_angles[direction] = angle_rad

# return the best direction allowing to move from previous_center to spot_center
def get_best_motors_direction(previous_center,spot_center):
    wanted_angle_rad = math.atan2(spot_center[1]-previous_center[1],spot_center[0]-previous_center[0])
    best_angle_diff = 2*math.pi
    best_direction = MotorsDirection.LEFT
    for direction, angle_rad in direction_angles.items():
        angle_diff = compare_angles(angle_rad,wanted_angle_rad)
        if angle_diff<best_angle_diff:
            best_angle_diff = angle_diff
            best_direction = direction
    prn(f"get_best_motors_direction: {angle_rad} rad -> {direction}")
    return direction

def reach_target():
    print("reach_target")
    direction = MotorsDirection.LEFT # todo : initialize to most used direction (most probable)

    previous_img = camera_capture()
    add_debug_image("previous",previous_img)
    draw_target()

    previous_spot_center = None
    target_error_px = 2*target_tol_px # force to run at least one iteration

    while target_error_px>target_tol_px:
        motors_move_one_step(direction)
        if wait_at_each_step:
            if update_ui(0):
                return
        else:
            for i in range(5):
                if update_ui(1000):
                    return
                get_motors_status() # todo :check 'locked'

        current_img = camera_capture()
        add_debug_image("current",current_img)

        spot_center = compute_spot_center(previous_img,current_img)
        if previous_spot_center is not None:
            update_motors_direction_history(direction,previous_spot_center,spot_center)
            direction = get_best_motors_direction(previous_spot_center,spot_center)
        previous_img = current_img
        previous_spot_center = spot_center

        target_error_px = math.dist(spot_center , target_pos)
        prn(f"target_error_px: {target_error_px}")


while True:
    current_img = camera_capture()
    add_debug_image("current",current_img)
    draw_target()

    if wait_at_each_step:
        update_ui(0)
    else:
        update_ui(10000)

    if tracking_enabled:
        reach_target()
    else:
        previous_img = current_img
        add_debug_image("previous",previous_img)



