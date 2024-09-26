# Copyright (C) 2024 Rémi Peuchot (https://remipch.github.io/)
# This code is distributed under GNU GPL v3 license

import sys
from remote_control_client import RemoteControlClient

arg_count = len(sys.argv)

if arg_count == 4:
    output_filename = sys.argv[1]

    first_month = int(sys.argv[2])
    assert first_month >= 1 and first_month <= 12

    last_month = int(sys.argv[3])
    assert last_month >= first_month and last_month <= 12

    # month index start from 0
    first_month = first_month - 1
    last_month = last_month - 1
else:
    print("Usage :")
    print("  python3 time_walker_recorder <output_filename> <first_month> <last_month>")
    print("With :")
    print("  first_month>=1 and first_month<=12 and last_month>=first_month and last_month<=12")
    exit(0)

remote_control_client = RemoteControlClient()

for date in range(first_month * 3, (last_month + 1) * 3):
    for time in range(0, 24 * 4):
        reply = remote_control_client.request(
            {
                "date_in_ten_days_count": date,
                "time_in_fifteen_minutes_count": time,
            },
            [
                "sun_azimuth_in_degree",
                "measure_area_power_in_watt",
            ],
        )
        print("Date: %i ; Time: %i ; Reply: %s" %
              (date, time, reply), flush=True)
