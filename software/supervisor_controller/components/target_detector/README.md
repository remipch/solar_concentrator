# target_detector

The goal of target_detector is to detect the rectangle area that will be considered as the target to sun rays.

To accomplish this, it uses a modified version of QR-code detector (quirc) that detect corner capstones.

From the detected capstone positions in image, it applies an hard-coded geometric pattern to compute the rectangle area.

Each capstones is detected with a width and a height :

        w
      ◄───►
   ▲  ┌───┐
  h│  │   │
   ▼  └───┘


If 4 capstones are successfully detected, the following pattern is applied from capstones :

            ┌───┐               ┌───┐─ ─▲
        ▲─ ─│ 1 │               │ 2 │   │h
        │   └───┘               └───┘─ ─▼
     2*h│   │                       │
        │   │                       │
        ▼─ ─┌───────────────────────┐
            │                       │
            │        target         │
            │         area          │
            │       rectangle       │
            │                       │
       ─▲─ ─└───────────────────────┘
        │   │                       │
     2*h│   │                       │
        │   ┌───┐               ┌───┐
        ▼─ ─│ 3 │               │ 4 │
            └───┘               └───┘


Given that capstone positions are not strictly aligned vertically and horizontally in real images,
the area rectangle is adjusted to always fit in the rectangle shown above :

  left_border = max(corner_1.left, corner_3.left)
  right_border = min(corner_2.right, corner_4.right)
  top_border = max(corner_1.center_y, corner_2.center_y) + 2 * average_corner_height
  bottom_border = min(corner_3.center_y, corner_4.center_y) - 2 * average_corner_height

This simple approch is valid as long as the perspective effect is negligible in images :
- the real target surface is almost perpendicular to the camera optical axis
- target and camera are almost horizontal

Moreover, a few simple harcoded checks are applied :
- vertical and horizontal misalignments must be less than minimal capstone side size
- rectangle size must be greater than maximal capstone side size
