use <flat_profile.scad>
use <panel_vertical_axis.scad>
use <panel_horizontal_axis.scad>
use <left_hinge.scad>
use <square_tube.scad>
use <small_hinge.scad>

GAP = 20;

SMALL_HINGE_ANGLE = 120;

EXPLODED = true;

module panel_frame(small_hinge_angle, exploded, gap) {
  panel_vertical_axis(false, gap);

  translate([0,0,panel_vertical_axis_length()-square_tube_width()]) {
    panel_horizontal_axis(small_hinge_angle, exploded, gap);

//     translate([0,0,square_tube_width()+(EXPLODED?GAP:0)])
//       small_hinge_move_to_other_half_origin(SMALL_HINGE_ANGLE)
//         translate([-200,-panel_horizontal_axis_length(),(exploded?-gap:0)-10])
//           %cube([400,panel_horizontal_axis_length(),10]);
  }
}

panel_frame(SMALL_HINGE_ANGLE, EXPLODED, GAP);
