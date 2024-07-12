use <bolt_and_nut.scad>
use <assembly.scad>
use <small_bracket.scad>
use <motor_block.scad>
use <stand_front_board.scad>
use <pulley.scad>
use <truncate.scad>

$fa = 10;
$fs = 0.1;

GAP = 20;

EXPLODED_AND_TRUNCATED = false;

mirror_width = 150;
mirror_depth = 4;
mirror_height = 150;

mirror_holder_width = 130;
mirror_holder_depth = 10;
mirror_holder_height = 130;
mirror_holder_offset_y = 15;

mirror_columns = 4;
mirror_rows = 6;

mirror_period_x = mirror_width + 5;
mirror_period_z = mirror_height + 5;

panel_board_width = mirror_columns * mirror_period_x;
panel_board_depth = 15;
panel_board_height = mirror_rows * mirror_period_z;

module mirror_holder(exploded) {
  difference() {
    translate([-mirror_holder_width/2,-mirror_holder_depth,-mirror_holder_height/2])
      cube([mirror_holder_width,mirror_holder_depth,mirror_holder_height]);

    translate([x,-1,z])
      rotate([-90,0,0])
        #cylinder(mirror_holder_depth+2,r=2);
  }
}

module panel_board(exploded=false, gap=GAP) {
  difference() {
    translate([-panel_board_width/2,0,-panel_board_height/2])
      cube([panel_board_width,panel_board_depth,panel_board_height]);

    for (c=[0:mirror_columns-1]) {
      x = mirror_period_x * (-(mirror_columns-1)/2 + c);
      for (r=[0:mirror_rows-1]) {
        z = mirror_period_z * (-(mirror_rows-1)/2 + r);
        translate([x,-1,z])
          rotate([-90,0,0])
            #cylinder(panel_board_depth+2,r=2);
      }
    }
  }

  for (c=[0:mirror_columns-1]) {
    x = mirror_period_x * (-(mirror_columns-1)/2 + c);
    angle_z = -x/50; // not the exact angle, just to show some mirror orientation
    for (r=[0:mirror_rows-1]) {
      z = mirror_period_z * (-(mirror_rows-1)/2 + r);
      angle_x = z/50; // not the exact angle, just to show some mirror orientation
      translate([x,-1,z])
        rotate([angle_x,0,angle_z])
          mirror_holder(exploded);
    }
  }
}

function stand_vertical_axis_length() = 600;

function back_board_width() = 1000;
function back_board_depth() = 15;
function back_board_height() = front_board_height();

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

    for (x=[-motor_block_offset_x,motor_block_offset_x]) {
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

if(EXPLODED_AND_TRUNCATED)
  truncate_negative_x([1100,600,400])
    stand_back_board(true, GAP);
else
//   stand_back_board(false);
  panel_board(false);
