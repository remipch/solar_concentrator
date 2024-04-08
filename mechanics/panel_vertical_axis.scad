use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>
use <flat_profile.scad>
use <panel_horizontal_axis.scad>

$fa = 3;
$fs = 0.4;

function panel_vertical_axis_length() = 200;

GAP = 20;

EXPLODED = true;

LINE_RADIUS = 0.01; // if exploded=true, pices alignment are shown with "cylinder" lines of this radius

function panel_vertical_axis_origin_to_hinges_t() = [[0,45,square_tube_width()],[0,panel_vertical_axis_length()-45,square_tube_width()]];

module panel_vertical_axis(exploded=false, gap=0) {
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
  }

  module hinge_bolt_assembly(bolt_t) {
    short_bolt = (bolt_t[1]>panel_vertical_axis_length()-20); // specific case for the last bolt : shorter because another bolt will be in the same axis
    bolt_length = short_bolt ? 15 : 40;
    bolt_gap = exploded?40+2*gap:0;
    bolt_z = bolt_gap + hinge_depth() + 1;
    washer_y = exploded ? (short_bolt ? 1.5*gap : 0) : 0;
    washer_gap = exploded ? (short_bolt ? -2*gap : -gap) : 0;
    washer_z = washer_gap + (short_bolt ? -square_tube_depth()-washer_m4_height():-square_tube_width()-washer_m4_height());
    nut_gap = exploded ? (short_bolt ? -3*gap : -2*gap) : 0;
    nut_z = nut_gap + (short_bolt ? -square_tube_depth()-washer_m4_height()-nut_m4_height(): -square_tube_width()-washer_m4_height()-nut_m4_height());
    nut_y = exploded ? (short_bolt ? 1.5*gap : 0) : 0;
    translate(bolt_t) {
      translate([0,0,bolt_z]) {
        countersunk_bolt_m4(bolt_length);
        if(exploded)
          rotate([180,0,0])
            cylinder(bolt_z, r=LINE_RADIUS);
      }

      translate([0,washer_y,washer_z]) {
        washer_m4();
        if(exploded)
          if(short_bolt)
            rotate([36.6,0,0])
              cylinder(2.5*gap, r=LINE_RADIUS);
          else
            cylinder(-washer_gap, r=LINE_RADIUS);
      }

      translate([0,nut_y,nut_z]) {
        nut_m4();
        if(exploded)
          cylinder(gap, r=LINE_RADIUS);
      }
    }
  }

  for (hinge_t=panel_vertical_axis_origin_to_hinges_t()) {
    for (hole_t=hinge_origin_to_holes_t()) {
        hinge_bolt_assembly(hinge_t+hole_t);
    }
  }

  // Round head bolt to fix diagonal bar
  translate([square_tube_width()/2,panel_vertical_axis_length()/2,square_tube_width()]) {
    translate([0,0,(exploded?30+gap:0)]) {
      round_head_bolt_m4(30);
      if(exploded)
        rotate([180,0,0])
          cylinder(hinge_depth()+square_tube_width()+ 30 + 5*gap, r=LINE_RADIUS);
    }

    translate([0,0,+(exploded?-3*gap:0)-square_tube_width()-flat_profile_depth()-washer_m4_height()])
      washer_m4();

    translate([0,0,(exploded?-4*gap:0)-square_tube_width()-flat_profile_depth()-washer_m4_height()-nut_m4_height()])
      nut_m4();
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

rotate([90,0,0])
  panel_vertical_axis(EXPLODED, GAP);
