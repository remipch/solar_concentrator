use <bolt_and_nut.scad>
use <assembly.scad>
use <stand_front_board.scad>
use <stand_back_board.scad>
use <small_bracket.scad>
use <left_hinge.scad>
use <square_tube.scad>

$fa = 10;
$fs = 0.1;

GAP = 100;

EXPLODED = true;

module stand_at_vertical_axis(exploded=false) {
  translate([0,front_board_length()/2 + (exploded?2*GAP:0),0])
    stand_back_board();

  stand_front_board();

  for (z=[stand_board_bracket_offset(),front_board_height() - stand_board_bracket_offset()]) {
    for (x=small_bracket_hole_offsets()) {
      translate([-x-front_board_width(),front_board_length()/2-small_bracket_depth()-1,z]) {
        rotate([90,0,0]) {
          simple_assembly(15,exploded=exploded,gap=GAP,extra_line_length=2*GAP)
            wood_screw_d4(15);
        }
      }
    }
  }
}

// centered at vertical hinges axis
module stand(exploded=false) {
  translate([hinge_depth(),square_tube_width()+hinge_male_origin_to_axis_t().x,0])
    stand_at_vertical_axis(exploded);
}

stand(EXPLODED);

