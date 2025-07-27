# Track
10m tracking code and GUI by Kevin Harris of SAO (this version from circa 2004)

## Screenshots from the GUI

![Object selection GUI](https://github.com/Whipple10m/Track/blob/main/Assets/tracking.png)

![Object selection GUI](https://github.com/Whipple10m/Track/blob/main/Assets/tracking2a.png)

![Object selection GUI](https://github.com/Whipple10m/Track/blob/main/Assets/tracking_objects.png)

## Original README

This directory has a tracking program with a new corrections routine
written by KRH using TPOINT information. It no longer takes the corrections from 
~/bin/10meter_corrections.txt

v1.93 includes changes:
1. the store direction information in: /home/observer/bin/.track_start_direction
2. auto-alignment position stored in: /home/observer/bin/.track_alignment_position
3. will object hit limit before setting??
4. more accurate position information using motor info
5. more information about controller status.
6. program version number on screen
7. Change "drift" to "position"
8. Get rid of moon tracking software
9. Add more setup info for controllers
