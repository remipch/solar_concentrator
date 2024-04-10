use <bolt_and_nut.scad>
use <square_tube.scad>
use <assembly.scad>
use <small_bracket.scad>
use <motor_block.scad>
use <stand_front_board.scad>

$fa = 3;
$fs = 0.4;

function stand_vertical_axis_length() = 600;

function back_board_width() = 1000;
function back_board_depth() = 15;
function back_board_height() = 150;

GAP = 20;

EXPLODED = true;

stand_pulley_offset_x = 250;
stand_pulley_offset_z = 40;

module stand_back_board(exploded=false, gap=GAP) {
  difference() {
    translate([-front_board_length()/2,0,0])
      cube([back_board_width(),back_board_depth(),back_board_height()]);

    for (x=[-stand_pulley_offset_x,stand_pulley_offset_x]) {
      translate([x,-1,stand_pulley_offset_z])
        rotate([-90,0,0])
          cylinder(back_board_depth()+2,r=3);
    }
  }
}

translate([0,front_board_length()/2,0])
  stand_back_board(EXPLODED, GAP);

%stand_front_board(EXPLODED, GAP);
