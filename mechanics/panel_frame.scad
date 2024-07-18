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

module panel_frame(small_hinge_angle, exploded, gap) {
  panel_vertical_axis(false, gap);

  translate([0,0,panel_vertical_axis_length()-square_tube_width()]) {
    translate([0,exploded?2*gap:0,0]) {
      panel_horizontal_axis(small_hinge_angle, exploded, gap);

//     translate([0,0,square_tube_width()+(EXPLODED?GAP:0)])
//       small_hinge_move_to_other_half_origin(SMALL_HINGE_ANGLE)
//         translate([-200,-panel_horizontal_axis_length(),(exploded?-gap:0)-10])
//           %cube([400,panel_horizontal_axis_length(),10]);
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

panel_frame(SMALL_HINGE_ANGLE, EXPLODED, GAP);
