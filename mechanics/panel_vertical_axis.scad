use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>
use <flat_profile.scad>
use <panel_horizontal_axis.scad>
use <assembly.scad>

$fa = 3;
$fs = 0.4;

function panel_vertical_axis_length() = 200;

GAP = 20;

EXPLODED = true;

function panel_vertical_axis_origin_to_hinges_t() = [[0,45,square_tube_width()],[0,panel_vertical_axis_length()-45,square_tube_width()]];

// Work along y axis because it's the natural orientation of internal objects
module panel_vertical_axis_along_y(exploded=false, gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(panel_vertical_axis_length());
    // Holes for left hinge
    for (hinge_t=panel_vertical_axis_origin_to_hinges_t()) {
      for (hole_t=hinge_origin_to_holes_t()) {
        t = hinge_t + hole_t;
        short_bolt = (t[1]>panel_vertical_axis_length()-20); // specific case for the last bolt : shorter because another bolt will be in the same axis
        translate(t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+(short_bolt?-5:2),2,2);
      }
    }

    // Hole for rounded head bolts
    translate([square_tube_width()/2,panel_vertical_axis_length()/2,-1])
      cylinder(square_tube_width()+2,2,2);
    translate([square_tube_width()/2,panel_vertical_axis_length()-square_tube_width()/2,-1])
      cylinder(square_tube_depth()+2,2,2);
  }

  for (hinge_t=panel_vertical_axis_origin_to_hinges_t()) {
    translate([0,0,exploded?gap:0])
      translate(hinge_t)
        left_hinge_female();

    for (hole_t=hinge_origin_to_holes_t()) {
      bolt_t = hinge_t+hole_t+[0,0,hinge_depth()];
      translate(bolt_t) {
        if(bolt_t[1]>panel_vertical_axis_length()-20) {
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

  // Round head bolt to fix diagonal bar
  translate([square_tube_width()/2,panel_vertical_axis_length()/2,square_tube_width()]) {
    bolt_assembly_m4(bolt_length=30, assembly_depth=square_tube_width()+flat_profile_depth(), washer_gap_z=3*GAP, exploded=exploded)
      round_head_bolt_m4(30);
  }

  module diagonal_bar() {
    diagonal_bar_hole_offset = 7; // offset from flat profile border to hole
    diagonal_bar_hole_distance_x = diagonal_fixpoint_offset_from_center_of_vertical_bar();
    diagonal_bar_hole_distance_y = panel_vertical_axis_length()/2 - square_tube_width()/2;
    diagonal_bar_angle = atan(diagonal_bar_hole_distance_y/diagonal_bar_hole_distance_x);
    diagonal_bar_hole_distance = sqrt(diagonal_bar_hole_distance_x^2 + diagonal_bar_hole_distance_y^2);
    diagonal_bar_gap = exploded?-2*gap:0;
    rotate([0,0,diagonal_bar_angle])
      translate([0,0,diagonal_bar_gap])
        difference() {
          translate([-diagonal_bar_hole_offset,-flat_profile_width()/2,0])
            flat_profile_bended(diagonal_bar_hole_distance + 2 * diagonal_bar_hole_offset);
          translate([0,0,-flat_profile_depth()-1])
            cylinder(2*flat_profile_depth()+2,2,2);
          translate([diagonal_bar_hole_distance,0,-flat_profile_depth()-1])
            cylinder(2*flat_profile_depth()+2,2,2);
        }
  }
  translate([square_tube_width()/2,panel_vertical_axis_length()/2,0]) {
    diagonal_bar();
  }
}

// Vertical axis in its final orientation
module panel_vertical_axis(exploded=false, gap=0) {
  rotate([90,0,0])
    panel_vertical_axis_along_y(EXPLODED, GAP);
}

panel_vertical_axis(EXPLODED, GAP);
