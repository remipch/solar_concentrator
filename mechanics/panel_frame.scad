use <flat_profile.scad>
use <panel_vertical_axis.scad>
use <panel_horizontal_axis.scad>
use <left_hinge.scad>
use <square_tube.scad>
use <small_hinge.scad>

GAP = 20;

SMALL_HINGE_ANGLE = 60;

EXPLODED = true;

function square_tube_width() = 23.5;

function square_tube_depth() = 1.5;

module panel_frame(exploded, gap) {
  rotate([90,0,0])
//     translate(-hinge_origin_to_axis_t() + [0,0,-square_tube_width()])
      panel_vertical_axis(exploded, gap);

  translate([-panel_horizontal_axis_length()/2,square_tube_width(),panel_vertical_axis_length()-square_tube_width()]) {
    rotate([0,0,-90]) {
      panel_horizontal_axis(exploded, gap);

//     translate([0,0,square_tube_width()+(EXPLODED?GAP:0)])
//       small_hinge_move_to_other_half_origin(SMALL_HINGE_ANGLE)
//         translate([-200,-panel_horizontal_axis_length(),(exploded?-gap:0)-10])
//           %cube([400,panel_horizontal_axis_length(),10]);
    }
  }
}

panel_frame(EXPLODED, GAP);
