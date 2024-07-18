
use <assembly.scad>
use <panel.scad>
use <panel_vertical_axis.scad>
use <stand.scad>
use <stand_front_board.scad>

GAP = 400;

EXPLODED = false;

VERTICAL_HINGE_ANGLE = 15;

SMALL_HINGE_ANGLE = 75;

module solar_panel(vertical_hinge_angle, small_hinge_angle, exploded=false) {
  stand();
  translate([0,0,stand_vertical_axis_length() - panel_vertical_axis_length() + (exploded?GAP:0)])
    rotate([0,0,vertical_hinge_angle])
      panel(small_hinge_angle);
  if(exploded) {
    translate([0,0,450])
      cylinder(GAP, r=exploded_line_radius());
  }
}

solar_panel(VERTICAL_HINGE_ANGLE, SMALL_HINGE_ANGLE, EXPLODED);
