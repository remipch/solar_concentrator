import cv2
import numpy as np

#blue = (255,50,50)
#orange = (0,164,255)
red = (0,0,204)
cyan = (204,204,0)
yellow = (0,204,204)

# tolerance arround target
target_tol_px = 20

# defined by Ctrl+Clic on image
target_pos = (100,100)

# threshold to detect start blobs and finish blobs from diff image
removed_hot_blob_max_level = 45
new_hot_blob_min_level = 220

# dictionary to map img name to image
debug_images = {}

def p(text,val):
    print(f"{text}: {val}",flush=True)

def on_clic(event,x,y,flags,img_name):
    global target_pos

    if event == cv2.EVENT_LBUTTONDOWN and flags & cv2.EVENT_FLAG_CTRLKEY:
        print(f"Ctrl+Clic image {img_name} at line {y}",flush=True)
        img = debug_images[img_name]
        img[y,x] = cyan
        cv2.imshow(img_name, img)

# draw a point in all debug images
def draw_point(x,y,color):
    for img_name, img in debug_images.items():
        img[int(y),int(x)] = color
        cv2.imshow(img_name, img)

def add_debug_image(img_name,img):
    print(f"add debug image {img_name}",flush=True)
    debug_img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    cv2.imshow(img_name, debug_img)
    cv2.setMouseCallback(img_name, on_clic, img_name)
    debug_images[img_name] = debug_img
    return debug_img

def add_image(img_name,img):
    print(f"add image {img_name}",flush=True)
    cv2.imshow(img_name, img)
    cv2.setMouseCallback(img_name, on_clic, img_name)
    debug_images[img_name] = img
    return img

def load(img_path):
    img = cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
    #img = cv2.divide(img, 2)
    p(f"{img_path} mean",cv2.mean(img))
    return img

def compute_blob_centroid(blob_img):
    M = cv2.moments(blob_img)
    if M["m00"] == 0:
        raise Exception("Cannot compute blob centroid")
    x = int(M["m10"] / M["m00"])
    y = int(M["m01"] / M["m00"])
    return x,y

def compute_spot_center(previous_img,current_img):
    add_debug_image("previous",previous_img)
    add_debug_image("current",current_img)

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
    p("spot_center_x",spot_center_x)
    p("spot_center_y",spot_center_y)

    draw_point(removed_blob_x,removed_blob_y,cyan)
    draw_point(new_blob_x,new_blob_y,red)
    draw_point(spot_center_x,spot_center_y,yellow)

b = load("B1.jpg")

c = load("C1.jpg")

compute_spot_center(b,c)

# opencv UI event loop
while True:
    if cv2.waitKey(5)==27:
        # exit on escape key
        break
