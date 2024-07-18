use <flat_profile.scad>
use <panel_vertical_axis.scad>
use <panel_horizontal_axis.scad>
use <left_hinge.scad>
use <square_tube.scad>
use <small_hinge.scad>
use <bolt_and_nut.scad>
use <assembly.scad>

GAP = 20;

SMALL_HINGE_ANGLE = 60;

EXPLODED = true;

FAKE_PANEL_BOARD = true;

FAKE_PANEL_BOARD_SCREW = true;

// "panel_frame_construction" use the default position and orientation of horizontal and verical axes
// to make the panel frame construction easier to understand
// But "panel_frame" module use the position and orientation that is easier to use by the "panel" module
// children(0) is panel board
// children(1) is wood screw to assemble panel board
module panel_frame_construction(small_hinge_angle, exploded) {
  panel_vertical_axis();

  translate([0,0,panel_vertical_axis_length()-square_tube_width()]) {
    translate([0,exploded?2*GAP:0,0]) {
      panel_horizontal_axis(small_hinge_angle)
        children(1);

      translate([0,square_tube_width(),square_tube_width()])
        rotate([0,0,-90])
          small_hinge_move_to_other_half_origin(small_hinge_angle)
            children(0);
    }

    translate([horizontal_axis_center_to_center_fixpoint(),square_tube_width(),square_tube_width()/2])
      rotate([-90,180,0])
          bolt_assembly_m4(bolt_length=30, bolt_gap_z=3*GAP, assembly_depth=square_tube_width()+square_tube_depth(), washer_gap_y=3*GAP, washer_gap_z=4*GAP, exploded=exploded)
            round_head_bolt_m4(30);

    translate([horizontal_axis_center_to_diagonal_fixpoint(),square_tube_width(),square_tube_width()/2])
      rotate([-90,0,0])
        bolt_assembly_m4(bolt_length=30, bolt_gap_z=3*GAP, assembly_depth=square_tube_width()+flat_profile_depth(), washer_gap_z=3*GAP, exploded=exploded)
          round_head_bolt_m4(30);
  }
}

// Rotate and move 'panel_frame_construction' at verical hinge axis
module panel_frame(small_hinge_angle, exploded=false) {
  rotate([0,0,180])
    translate([-hinge_origin_to_axis_t().x,hinge_depth()+square_tube_width(),0])
      panel_frame_construction(small_hinge_angle, exploded) {
        children(0);
        children(1);
      }
}

panel_frame(SMALL_HINGE_ANGLE, EXPLODED) {
  if(FAKE_PANEL_BOARD) {
    translate([-panel_horizontal_axis_length()/2,0,-100])
      %cube([panel_horizontal_axis_length(),20,200]);
  }
  if(FAKE_PANEL_BOARD_SCREW) {
    %wood_screw_d4(15);
  }
}
