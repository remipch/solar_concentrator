
use <solar_panel.scad>

min_vertical_hinge_angle = -60;
max_vertical_hinge_angle = 60;
min_small_hinge_angle = 20;
max_small_hinge_angle = 90;

if($t<0.25) {
  vertical_hinge_angle = min_vertical_hinge_angle + $t*(max_vertical_hinge_angle-min_vertical_hinge_angle) / 0.25;
  small_hinge_angle = max_small_hinge_angle;
  solar_panel(vertical_hinge_angle, small_hinge_angle);
}
else if($t<0.5) {
  vertical_hinge_angle = max_vertical_hinge_angle;
  small_hinge_angle = max_small_hinge_angle - ($t-0.25) * (max_small_hinge_angle-min_small_hinge_angle) / 0.25;
  solar_panel(vertical_hinge_angle, small_hinge_angle);
}
else if($t<0.75) {
  vertical_hinge_angle = max_vertical_hinge_angle - ($t-0.5)*(max_vertical_hinge_angle-min_vertical_hinge_angle) / 0.25;
  small_hinge_angle = min_small_hinge_angle;
  solar_panel(vertical_hinge_angle, small_hinge_angle);
}
else {
  vertical_hinge_angle = min_vertical_hinge_angle;
  small_hinge_angle = min_small_hinge_angle + ($t-0.75) * (max_small_hinge_angle-min_small_hinge_angle) / 0.25;
  solar_panel(vertical_hinge_angle, small_hinge_angle);
}
