use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>
use <flat_profile.scad>
use <panel_horizontal_axis.scad>
use <panel_vertical_axis.scad>
use <assembly.scad>

$fa = 3;
$fs = 0.4;

function stand_vertical_axis_length() = 600;

GAP = 20;

EXPLODED = true;

LINE_RADIUS = 0.01; // if exploded=true, pices alignment are shown with "cylinder" lines of this radius

function stand_vertical_axis_origin_to_hinges_t() = [
  for (hinge_t = panel_vertical_axis_origin_to_hinges_t())
    hinge_t + [square_tube_width(),stand_vertical_axis_length()-panel_vertical_axis_length(),0]
];

function stand_vertical_axis_origin_to_stand_holes_t() = [[square_tube_width()/2,30,0],[square_tube_width()/2,130,0]];

// Work along y axis because it's the natural orientation of internal objects
module stand_vertical_axis_along_y(exploded=false, gap=0) {
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

  module stand_bolt_assembly(bolt_t) {
    short_bolt = (bolt_t[1]>stand_vertical_axis_length()-20); // specific case for the last bolt : shorter because another bolt will be in the same axis
    bolt_gap = exploded?-60-2*gap:0;
    bolt_z = bolt_gap - 1;
    washer_gap = exploded ? gap : 0;
    washer_z = washer_gap + square_tube_width();
    nut_gap = exploded ? 2*gap : 0;
    nut_z = nut_gap + square_tube_width()+washer_m6_height();
    translate(bolt_t) {
      translate([0,0,bolt_z]) {
        rotate([0,180,0])
          countersunk_bolt_m6(60);
        if(exploded)
          cylinder(-bolt_z, r=LINE_RADIUS);
      }

      translate([0,0,washer_z]) {
        washer_m6();
        if(exploded)
          rotate([0,180,0])
            cylinder(washer_gap, r=LINE_RADIUS);
      }

      translate([0,0,nut_z]) {
        nut_m6();
        if(exploded)
          rotate([0,180,0])
            cylinder(nut_gap, r=LINE_RADIUS);
      }
    }
  }

  // Hole for stand assembly
  for (bolt_t=stand_vertical_axis_origin_to_stand_holes_t()) {
    stand_bolt_assembly(bolt_t);
  }
}

// Horizontal axis in its final orientation
module stand_vertical_axis(exploded=false, gap=0) {
  rotate([90,0,-90])
    stand_vertical_axis_along_y(exploded, GAP);
}

//stand_vertical_axis_along_y(EXPLODED, GAP);

stand_vertical_axis(EXPLODED, GAP);


// translate([40,400,0])
//   panel_vertical_axis(EXPLODED, GAP);
