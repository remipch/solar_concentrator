use <bolt_and_nut.scad>
use <square_tube.scad>
use <assembly.scad>
use <small_bracket.scad>
use <motor_block.scad>
use <stand_front_board.scad>
use <pulley.scad>

$fa = 10;
$fs = 0.1;

function stand_vertical_axis_length() = 600;

function back_board_width() = 1000;
function back_board_depth() = 15;
function back_board_height() = front_board_height();

GAP = 30;

EXPLODED = true;

stand_pulley_offset_x = 250;
stand_pulley_offset_z = 40;

motor_block_offset_x = 60;
motor_block_offset_y = -motor_support_axis_offset();
motor_block_offset_z = 80;

bracket_offset_z = 15;

leg_width = 28;
leg_depth = 10;
leg_height = 200;
leg_hole_offset = 15;
leg_holes_t = [for (z = [leg_height-back_board_height()+leg_hole_offset,leg_height-leg_hole_offset]) [leg_width/2,-1,z] ];

module stand_back_board(exploded=false, gap=GAP) {
  difference() {
    translate([-front_board_length()/2,0,0])
      cube([back_board_width(),back_board_depth(),back_board_height()]);

    for (x=[-stand_pulley_offset_x,stand_pulley_offset_x]) {
      translate([x,-1,stand_pulley_offset_z])
        rotate([-90,0,0])
          cylinder(back_board_depth()+2,r=3);
    }

    for (x=[-motor_block_offset_x-back_board_depth(),motor_block_offset_x]) {
      translate([x,0,motor_block_offset_z]) {
        for (hole_t=motor_support_holes_t()) {
          translate([hole_t.x,-1,hole_t.z])
            rotate([-90,0,0])
              cylinder(back_board_depth()+2,r=2);
        }
      }
    }
  }

  for (x=[-stand_pulley_offset_x,stand_pulley_offset_x]) {
    translate([x,0,stand_pulley_offset_z])
      rotate([90,0,0])
        bolt_assembly_m6(bolt_length=40,assembly_depth=back_board_depth(),washer_gap_z=5*gap,nut_gap_z=2*gap,exploded=exploded)
          vertical_pulley(40);
  }

  for (x=[-motor_block_offset_x-back_board_depth(),motor_block_offset_x]) {
    translate([x,motor_block_offset_y-(exploded?5*gap:0),motor_block_offset_z]) {
      motor_block();

      for (hole_t=motor_support_holes_t()) {
        translate(hole_t)
          rotate([90,0,0])
            bolt_assembly_m4(bolt_length=40, assembly_depth=back_board_depth()+motor_support_depth(),washer_gap_z=7*gap,nut_gap_z=2*gap, exploded=exploded)
              round_head_bolt_m4(40);
      }
    }
    translate([x,exploded?-gap:0,bracket_offset_z]){
      small_bracket();
      for (hole_t=small_bracket_origin_to_vertical_holes_t()) {
        translate(hole_t + [0,-1,0])
          rotate([90,0,0])
            simple_assembly(15,exploded=exploded,extra_line_length=gap) {
              wood_screw_d4(15);
            }
      }
    }
  }

  for (x=[-back_board_width()/2,back_board_width()/2-leg_width]) {
    translate([x,-leg_depth-(exploded?2*gap:0),back_board_height() - leg_height]) {
      difference() {
        cube([leg_width,leg_depth,leg_height]);
        for (hole_t=leg_holes_t) {
          translate(hole_t)
            rotate([-90,0,0])
              cylinder(leg_depth+2,r=2);
        }
      }
      for (hole_t=leg_holes_t) {
        translate(hole_t)
          rotate([90,0,0])
            simple_assembly(20,exploded=exploded,extra_line_length=2*gap,gap=2*gap) {
              wood_screw_d4(20);
        }
      }
    }
  }
}

stand_back_board(EXPLODED, GAP);

