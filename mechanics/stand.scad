use <bolt_and_nut.scad>
use <assembly.scad>
use <stand_front_board.scad>
use <stand_back_board.scad>
use <small_bracket.scad>
use <flat_profile.scad>

$fa = 10;
$fs = 0.1;

GAP = 200;

EXPLODED = true;

front_board_diagonal_hole_y = 100;
back_board_diagonal_hole_x = 250;

front_board_diagonal_hole_t = [-front_board_width()/2,front_board_diagonal_hole_y,front_board_height()];
back_board_diagonal_hole_t = [back_board_diagonal_hole_x,front_board_length()/2+back_board_depth()/2,back_board_height()];

module diagonal_bar(exploded, gap) {

  diagonal_vector = back_board_diagonal_hole_t - front_board_diagonal_hole_t;

  diagonal_bar_angle = atan(-diagonal_vector.x/diagonal_vector.y);
  diagonal_bar_hole_distance = sqrt(diagonal_vector.x^2 + diagonal_vector.y^2);
  diagonal_bar_hole_offset = 8; // offset from flat profile border to hole

  rotate([0,0,diagonal_bar_angle]) {
    translate([0,0,exploded?gap:0]) {
      difference() {
        translate([-flat_profile_width()/2,-diagonal_bar_hole_offset,0])
          flat_profile(diagonal_bar_hole_distance + 2 * diagonal_bar_hole_offset);
        translate([0,0,-flat_profile_depth()-1])
          cylinder(2*flat_profile_depth()+2,2,2);
        translate([0,diagonal_bar_hole_distance,-flat_profile_depth()-1])
          cylinder(2*flat_profile_depth()+2,2,2);
      }
    }
  }
}

module stand(exploded=false, gap=GAP) {
  translate([0,front_board_length()/2 + (exploded?gap:0),0])
    stand_back_board();

  stand_front_board();

  for (z=[stand_board_bracket_offset(),front_board_height() - stand_board_bracket_offset()]) {
    for (x=small_bracket_hole_offsets()) {
      translate([-x-front_board_width(),front_board_length()/2-small_bracket_depth()-1,z]) {
        rotate([90,0,0]) {
          simple_assembly(15,exploded=exploded,gap=gap,extra_line_length=gap)
            wood_screw_d4(15);
        }
      }
    }
  }

  translate(front_board_diagonal_hole_t)
    diagonal_bar(exploded, gap);

  translate(front_board_diagonal_hole_t + [0,0,flat_profile_depth()+2+(exploded?gap:0)]) {
    simple_assembly(20,exploded=exploded,gap=gap,extra_line_length=gap)
      wood_screw_d4(20);
  }

  translate(back_board_diagonal_hole_t + [0,0,flat_profile_depth()+2+(exploded?gap:0)]) {
    simple_assembly(20,exploded=exploded,gap=gap,extra_line_length=gap/2-20)
      wood_screw_d4(20);

    if(exploded) {
      translate([0,0,-gap/2]) {
        rotate([-90,0,0])
          cylinder(gap, r=exploded_line_radius());
        translate([0,gap,0])
          rotate([180,0,0])
            cylinder(gap/2, r=exploded_line_radius());
      }
    }
  }
}

stand(EXPLODED, GAP);

