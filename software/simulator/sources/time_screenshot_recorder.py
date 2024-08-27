import sys
from remote_control_client import RemoteControlClient
from PIL import ImageGrab

arg_count = len(sys.argv)

if arg_count == 3:
    first_time = int(sys.argv[1])
    assert first_time >= 0 and first_time < 96

    last_time = int(sys.argv[2])
    assert last_time >= first_time and last_time < 96
else:
    print("Usage :")
    print("  python3 time_screenshot_recorder <first_time> <last_time>")
    print("With :")
    print("  first_time>=0 and first_time<96 and last_time>=first_time and last_time<96")
    exit(0)

remote_control_client = RemoteControlClient()

for time in range(first_time, last_time):
    reply = remote_control_client.request(
        {
            "time_in_fifteen_minutes_count": time,
        },
        [
            "sun_azimuth_in_degree",
            "measure_area_power_in_watt",
        ],
    )
    print("Time: %i ; Reply: %s" % (time, reply), flush=True)

    screenshot = ImageGrab.grab(bbox=(920, 20, 1920, 780)) #TODO get window pos and dim from its name
    screenshot.save(f"screenshot_{time:02d}.png")

    # Close the screenshot
    screenshot.close()
