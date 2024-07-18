
use <assembly.scad>

use <panel_board.scad>
use <panel_frame.scad>
use <panel_horizontal_axis.scad>
use <small_hinge.scad>
use <bolt_and_nut.scad>

GAP = 200;

EXPLODED = false;

SMALL_HINGE_ANGLE = 60;

panel_board_pos_z = 85;

module panel(small_hinge_angle, exploded=false) {
  panel_frame(small_hinge_angle) {
    rotate([0,0,180])
      translate([0,-panel_board_depth()-(exploded?GAP:0),panel_board_pos_z]){
        panel_board();
      }
    simple_assembly(15,exploded=exploded, gap=GAP, extra_line_length=GAP)
      wood_screw_d4(15);
  }
}

panel(SMALL_HINGE_ANGLE, EXPLODED);
