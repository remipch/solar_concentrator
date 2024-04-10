use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>
use <panel_vertical_axis.scad>
use <assembly.scad>
use <small_bracket.scad>

$fa = 3;
$fs = 0.4;

function stand_vertical_axis_length() = 600;

function front_board_width() = 15;
function front_board_length() = 1000;
function front_board_height() = 150;

GAP = 20;

EXPLODED = true;


function stand_vertical_axis_origin_to_hinges_t() = [
  for (hinge_t = panel_vertical_axis_origin_to_hinges_t())
    hinge_t + [square_tube_width(),stand_vertical_axis_length()-panel_vertical_axis_length(),0]
];

stand_vertical_axis_hole_offset = 30;

stand_board_bracket_offset = 15;

function stand_vertical_axis_origin_to_stand_holes_t() = [
  [square_tube_width()/2,stand_vertical_axis_hole_offset,0],
  [square_tube_width()/2,front_board_height()-stand_vertical_axis_hole_offset,0]
];

// Work along y axis because it's the natural orientation of internal objects
module stand_vertical_axis_along_y(exploded, gap) {
  // square tube with all required holes
  difference() {
    square_tube(stand_vertical_axis_length());
    // Holes for left hinge
    for (hinge_t=stand_vertical_axis_origin_to_hinges_t()) {
      for (hole_t=hinge_male_origin_to_holes_t()) {
        t = hinge_t + hole_t;
        short_bolt = (t[1]>stand_vertical_axis_length()-20); // specific case for the last bolt : shorter because another bolt will be in the same axis
        translate(t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+(short_bolt?-5:2),2,2);
      }
    }

    // Hole for stand assembly
    for (hole_t=stand_vertical_axis_origin_to_stand_holes_t()) {
      translate(hole_t + [0,0,-1])
        cylinder(square_tube_width()+2,3,3);
    }
  }

  for (hinge_t=stand_vertical_axis_origin_to_hinges_t()) {
    translate([0,0,exploded?gap:0])
      translate(hinge_t)
        left_hinge_male();

    for (hole_t=hinge_male_origin_to_holes_t()) {
      bolt_t = hinge_t+hole_t+[0,0,hinge_depth()];
      translate(bolt_t) {
        if(bolt_t[1]>stand_vertical_axis_length()-20) {
          // specific case for the last bolt : shorter because another bolt will be in the same axis
          bolt_assembly_m4(bolt_length=15, bolt_gap_z=3*GAP, assembly_depth=square_tube_depth()+hinge_depth(), washer_gap_y = 1.5*GAP, washer_gap_z = 2*GAP, exploded=exploded)
            countersunk_bolt_m4(15);
        }
        else {
          bolt_assembly_m4(bolt_length=40, bolt_gap_z=2*GAP, assembly_depth=square_tube_width()+hinge_depth(), exploded=exploded)
            countersunk_bolt_m4(40);
        }
      }
    }
  }

  // Bolts for stand assembly
  for (bolt_t=stand_vertical_axis_origin_to_stand_holes_t()) {
    translate(bolt_t)
      rotate([180,0,0])
        bolt_assembly_m6(bolt_length=60, bolt_gap_z=2*GAP, assembly_depth=square_tube_width()+front_board_width(), exploded=exploded)
          round_head_bolt_m6(50);
  }
}

module board(exploded, gap) {
  difference() {
    translate([-front_board_width(),-front_board_length()/2,0])
      cube([front_board_width(),front_board_length(),front_board_height()]);

    for (z=[stand_vertical_axis_hole_offset,front_board_height() - stand_vertical_axis_hole_offset]) {
      translate([-front_board_width()-1,-square_tube_width()/2,z])
        rotate([0,90,0])
          cylinder(front_board_width()+2,3,3);
    }
  }

  for (z=[stand_board_bracket_offset,front_board_height() - stand_board_bracket_offset]) {
    translate([-front_board_width()-(exploded?gap:0),front_board_length()/2,z]) {
      rotate([0,-90,0]) {
        small_bracket();

        for (hole_t=small_bracket_origin_to_holes_t()) {
          translate(hole_t + [0,0,small_bracket_depth()+1])
            simple_assembly(15,exploded=exploded,gap=3*gap,extra_line_length=gap)
              wood_screw_d4(15);
        }
      }
    }
  }
}

module stand_front_board(exploded=false, gap=GAP) {
  translate([square_tube_width(),0,0])
    rotate([90,0,-90])
      stand_vertical_axis_along_y(exploded, gap);

  board(exploded, gap);
}

stand_front_board(EXPLODED, GAP);
