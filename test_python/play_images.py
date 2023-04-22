import cv2
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use('TkAgg')

# interactive mode is required to let opencv UI and matplotlib UI live in peace together
# (see main loop at the end of this file)
plt.ion()

blue = (255,50,50)
red = (50,50,255)
orange = (0,164,255)
cyan = (255,192,50)

# working area width and height around min and max
area_size_px = 40

# threshold to detect start blobs and finish blobs from diff image
start_blob_max_level = 45
finish_blob_min_level = 220

fig = None

# dictionaries to map img name to usefull related objects
images = {}
plots = {}

def p(text,val):
    print(f"{text}: {val}",flush=True)

def on_clic(event,x,y,flags,img_name):
    global fig,ax,plots,images

    if event == cv2.EVENT_LBUTTONDBLCLK:
        print(f"double click image {img_name} at line {y}",flush=True)
        img = images[img_name]
        y_line = img[y,:]

        if fig is None:
            fig = plt.figure()
            ax = fig.add_subplot(111)
            plt.legend(loc="upper left")

        if img_name in plots:
            plot = plots[img_name]
            plot.set_ydata(y_line)
            fig.canvas.draw()
            fig.canvas.flush_events()
            #plt.plot(y_line)
            #plt.show()
        else:
            plots[img_name] = ax.plot(y_line, label=img_name)
            plt.legend(loc="upper left")



def add_image(img_name,img):
    print(f"add image {img_name}",flush=True)
    cv2.imshow(img_name, img)
    cv2.setMouseCallback(img_name, on_clic, img_name)
    images[img_name] = img
    return img

def load(img_path):
    img = cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
    img = cv2.divide(img, 2)
    p(f"{img_path} mean",cv2.mean(img))
    return img

b = load("B1.jpg")

c = load("C1.jpg")

diff = (128+c)-b
add_image("diff",diff)

diff_norm = cv2.normalize(diff, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX)
add_image("diff_norm",diff_norm)
cv2.imwrite("diff_norm.png",diff_norm)

diff_map = cv2.applyColorMap(diff_norm, cv2.COLORMAP_JET)
cv2.imshow("diff_map", diff_map)
cv2.imwrite("diff_map.png",diff_map)

diff_color = cv2.cvtColor(diff_norm, cv2.COLOR_GRAY2BGR)
min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(diff_norm)

min_x = min_loc[0]
min_y = min_loc[1]
max_x = max_loc[0]
max_y = max_loc[1]

b_color = cv2.cvtColor(b, cv2.COLOR_GRAY2BGR)
c_color = cv2.cvtColor(c, cv2.COLOR_GRAY2BGR)

b_color[min_y,min_x] = blue
c_color[min_y,min_x] = blue
diff_color[min_y,min_x] = blue

b_color[max_y,max_x] = red
c_color[max_y,max_x] = red
diff_color[max_y,max_x] = red

area_center_x = (min_x+max_x)/2
area_center_y = (min_y+max_y)/2

start_blobs = cv2.threshold(diff_norm, start_blob_max_level, 255, cv2.THRESH_TOZERO_INV)[1]
start_blobs_color = cv2.cvtColor(start_blobs, cv2.COLOR_GRAY2BGR)

M = cv2.moments(start_blobs)
if M["m00"] == 0:
    raise Exception("Cannot compute centroid")
start_x = int(M["m10"] / M["m00"])
start_y = int(M["m01"] / M["m00"])
diff_color[start_y,start_x] = cyan
start_blobs_color[start_y,start_x] = cyan

finish_blobs = cv2.threshold(diff_norm, finish_blob_min_level, 255, cv2.THRESH_TOZERO)[1]
finish_blobs_color = cv2.cvtColor(finish_blobs, cv2.COLOR_GRAY2BGR)

M = cv2.moments(finish_blobs)
if M["m00"] == 0:
    raise Exception("Cannot compute centroid")
finish_x = int(M["m10"] / M["m00"])
finish_y = int(M["m01"] / M["m00"])
diff_color[finish_y,finish_x] = orange
finish_blobs_color[finish_y,finish_x] = orange

cv2.imshow("start_blobs_color", start_blobs_color)
cv2.imshow("finish_blobs_color", finish_blobs_color)
cv2.imshow("diff_color", diff_color)
add_image("b_color",b_color)
add_image("c_color",c_color)

# ugly way to run 2 UI event loops
# it make the plot flicker if another window is on top
while True:
    #global fig
    if not fig is None:
        # let matplotlib manage its UI
        plt.pause(0.005)

    # let opencv manage its UI
    if cv2.waitKey(5)==27:
        # exit on escape key
        break
