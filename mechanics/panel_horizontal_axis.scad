use <bolt_and_nut.scad>
use <square_tube.scad>
use <small_hinge.scad>
use <flat_profile.scad>

$fa = 3;
$fs = 0.4;

function panel_horizontal_axis_length() = 450;

GAP = 20;

SMALL_HINGE_ANGLE = 60;

EXPLODED = true;

LINE_RADIUS = 0.01; // if exploded=true, pieces alignment are shown with "cylinder" lines of this radius

function origin_to_hinges_t() = [[0,small_hinge_height()/2,square_tube_width()],[0,panel_horizontal_axis_length()-small_hinge_height()/2,square_tube_width()]];

function diagonal_fixpoint_offset_from_center_of_vertical_bar() = 100;

function origin_to_center_fixpoint_t() = [0,(square_tube_width()+panel_horizontal_axis_length())/2,square_tube_width()/2];

function origin_to_diagonal_fixpoint_t() = origin_to_center_fixpoint_t() + [0,diagonal_fixpoint_offset_from_center_of_vertical_bar(),0];

module panel_horizontal_axis(small_hinge_angle, exploded=false, gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(panel_horizontal_axis_length());
    for (hinge_t=origin_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=small_hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+2,2,2);
      }
    }
    for (hole_t=[origin_to_center_fixpoint_t(),origin_to_diagonal_fixpoint_t()]) {
      translate(hole_t + [-1,0,0])
        rotate([0,90,0])
          cylinder(square_tube_width()+2,2,2);
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate([0,0,exploded?gap:0])
      translate(hinge_t)
        small_hinge(small_hinge_angle);
  }

  module hinge_bolt_assembly() {
    translate([0,0,small_hinge_depth()+(exploded?40+2*gap:0)+1])
      countersunk_bolt_m4(40);

    translate([0,0,(exploded?-gap:0)-square_tube_width()-washer_m4_height()])
      washer_m4();

    translate([0,0,(exploded?-2*gap:0)-square_tube_width()-washer_m4_height()-nut_m4_height()]) {
      nut_m4();

      if(exploded)
        cylinder(small_hinge_depth()+square_tube_width()+40+4*gap, r=LINE_RADIUS);
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate(hinge_t)
      for (hole_t=small_hinge_origin_to_holes_t()) {
        translate(hole_t)
          hinge_bolt_assembly();
      }
  }

  module round_head_bolt_assembly(depth_offset, exploded_washer_offset_x, exploded_washer_offset_z) {
    bolt_x = exploded?-30-gap:0;
    translate([bolt_x,0,0]){
      rotate([0,-90,0])
        round_head_bolt_m4(30);
      if(exploded)
        rotate([0,90,0])
          cylinder(-bolt_x, r=LINE_RADIUS);
    }
    exploded_washer_offset_x = depth_offset + washer_m4_height() + (exploded ? exploded_washer_offset_x : 0);
    washer_x = square_tube_width() + exploded_washer_offset_x;
    washer_z = (exploded ? exploded_washer_offset_z : 0);

    translate([washer_x,0,washer_z]){
      rotate([0,-90,0]) {
        washer_m4();
        if(exploded) {
          angle = atan(exploded_washer_offset_z/exploded_washer_offset_x);
          line_length = sqrt(exploded_washer_offset_z^2 + exploded_washer_offset_x^2);
          rotate([0,-angle,0])
            cylinder(line_length, r=LINE_RADIUS);
        }
      }
    }

    nut_x = washer_x + nut_m4_height() + (exploded ? gap : 0);
    nut_z = washer_z;
    translate([nut_x,0,nut_z]) {
      rotate([0,-90,0]){
        nut_m4();

        if(exploded)
          cylinder(gap, r=LINE_RADIUS);
      }
    }
  }

  translate(origin_to_center_fixpoint_t())
    round_head_bolt_assembly(square_tube_depth(), 3*gap, 2.5*gap);
  translate(origin_to_diagonal_fixpoint_t())
    round_head_bolt_assembly(flat_profile_depth(), 2*gap, 0);
}

panel_horizontal_axis(SMALL_HINGE_ANGLE, EXPLODED, GAP);
