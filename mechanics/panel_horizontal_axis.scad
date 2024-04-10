use <bolt_and_nut.scad>
use <square_tube.scad>
use <small_hinge.scad>
use <flat_profile.scad>
use <assembly.scad>

$fa = 3;
$fs = 0.4;

function panel_horizontal_axis_length() = 450;

GAP = 20;

SMALL_HINGE_ANGLE = 60;

EXPLODED = true;

function horizontal_axis_origin_to_hinges_t() = [[0,small_hinge_height()/2,square_tube_width()],[0,panel_horizontal_axis_length()-small_hinge_height()/2,square_tube_width()]];

function diagonal_fixpoint_offset_from_center_of_vertical_bar() = 100;

function horizontal_axis_center_to_center_fixpoint() = square_tube_width()/2;

function horizontal_axis_center_to_diagonal_fixpoint() = horizontal_axis_center_to_center_fixpoint() + diagonal_fixpoint_offset_from_center_of_vertical_bar();

// Work along y axis because it's the natural orientation of internal objects
module panel_horizontal_axis_along_y(small_hinge_angle, exploded=false, gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(panel_horizontal_axis_length());
    for (hinge_t=horizontal_axis_origin_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=small_hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+2,2,2);
      }
    }
    for (hole_offset=[horizontal_axis_center_to_center_fixpoint(),horizontal_axis_center_to_diagonal_fixpoint()]) {
      translate([-1,panel_horizontal_axis_length()/2+hole_offset,square_tube_width()/2])
        rotate([0,90,0])
          cylinder(square_tube_width()+2,2,2);
    }
  }

  for (hinge_t=horizontal_axis_origin_to_hinges_t()) {
    translate(hinge_t) {
      translate([0,0,exploded?gap:0])
        small_hinge(small_hinge_angle);

      for (hole_t=small_hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,small_hinge_depth()])
          bolt_assembly_m4(bolt_length=40, bolt_gap_z = 2*GAP, assembly_depth=square_tube_width()+small_hinge_depth(), washer_gap_z=2*GAP, exploded=exploded)
            countersunk_bolt_m4(40);
      }
    }
  }
}

// Horizontal axis in its final orientation
module panel_horizontal_axis(small_hinge_angle, exploded=false, gap=0) {
  translate([-panel_horizontal_axis_length()/2,square_tube_width(),0])
    rotate([0,0,-90])
      panel_horizontal_axis_along_y(small_hinge_angle, exploded, GAP);
}

panel_horizontal_axis(SMALL_HINGE_ANGLE, EXPLODED, GAP);
