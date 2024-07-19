
use <assembly.scad>
use <panel_board.scad>
use <panel_frame.scad>
use <panel_horizontal_axis.scad>
use <small_hinge.scad>
use <bolt_and_nut.scad>
use <panel_vertical_axis.scad>
use <stand.scad>
use <stand_front_board.scad>

GAP = 300;

EXPLODED = false;

VERTICAL_HINGE_ANGLE = 15;

SMALL_HINGE_ANGLE = 75;

panel_board_pos_z = 85;

module panel(small_hinge_angle, exploded) {
  panel_frame(small_hinge_angle) {
    rotate([0,0,180])
      translate([0,-panel_board_depth()-(exploded?GAP:0),panel_board_pos_z]){
        panel_board();
      }
    simple_assembly(15,exploded=exploded, gap=GAP, extra_line_length=GAP)
      wood_screw_d4(15);
  }
}

module solar_panel(vertical_hinge_angle, small_hinge_angle, exploded=false) {
  stand();
  translate([0,0,stand_vertical_axis_length() - panel_vertical_axis_length() + (exploded?GAP:0)])
    rotate([0,0,vertical_hinge_angle])
      panel(small_hinge_angle, exploded);
  if(exploded) {
    translate([0,0,450])
      cylinder(GAP, r=exploded_line_radius());
  }
}

solar_panel(VERTICAL_HINGE_ANGLE, SMALL_HINGE_ANGLE, EXPLODED);
