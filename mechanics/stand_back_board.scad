use <bolt_and_nut.scad>
use <square_tube.scad>
use <assembly.scad>
use <small_bracket.scad>
use <motor_block.scad>
use <stand_front_board.scad>
use <pulley.scad>

$fa = 3;
$fs = 0.4;

function stand_vertical_axis_length() = 600;

function back_board_width() = 1000;
function back_board_depth() = 15;
function back_board_height() = front_board_height();

GAP = 20;

EXPLODED = true;

stand_pulley_offset_x = 250;
stand_pulley_offset_z = 40;

motor_block_offset_x = 60;
motor_block_offset_y = -motor_support_axis_offset();
motor_block_offset_z = 80;

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

  for (x=[-stand_pulley_offset_x,stand_pulley_offset_x]) {
    translate([x,0,stand_pulley_offset_z])
      rotate([90,0,0])
        bolt_assembly_m6(bolt_length=40,assembly_depth=back_board_depth(),washer_gap_z=3*gap,exploded=exploded)
          vertical_pulley(40);
  }

  for (x=[-motor_block_offset_x-back_board_depth(),motor_block_offset_x]) {
    translate([x,motor_block_offset_y,motor_block_offset_z])
      motor_block();
  }
}

translate([0,front_board_length()/2,0])
  stand_back_board(EXPLODED, GAP);

%stand_front_board();
