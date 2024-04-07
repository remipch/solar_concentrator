use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>

$fa = 3;
$fs = 0.4;

bottom_hinge_pos = 100;
top_hinge_pos = 300;

function center_to_hinges_t() = [[0,100,0],[0,300,0]];

GAP = 20;

exploded = true;

module panel_vertical_axis(gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(400);
    for (hinge_t=center_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=hinge_center_to_holes_t()) {
        translate(hole_t)
          cylinder(square_tube_width()+2,2,2);
      }
    }
  }

  translate([0,0,square_tube_width()+hinge_depth() ])
  for (hinge_t=center_to_hinges_t()) {
    translate(hinge_t)
    left_hinge_female();
  }

  module hinge_bolt_assembly() {
    translate([0,0,(exploded?40+2*gap:0)+square_tube_width()+hinge_depth()+2])
      countersunk_bolt_m4(40);

    translate([0,0,(exploded?-gap:0)-washer_m4_height()])
      washer_m4();

    translate([0,0,(exploded?-2*gap:0)-washer_m4_height()-nut_m4_height()])
      nut_m4();

    if(exploded) {
      color([0.8,0.8,0.8])
        translate([0,0,-2*gap])
          cube([0.01,0.01,40+4*gap]);
    }
  }

  for (y=[-hinge_holes_gap():hinge_holes_gap():hinge_holes_gap()]) {
    translate([hinge_holes_offset(),bottom_hinge_pos+y,0])
      hinge_bolt_assembly();
    translate([hinge_holes_offset(),top_hinge_pos+y,0])
      hinge_bolt_assembly();
  }

  translate([0,20,0])
    %round_head_bolt_m4(30);

  translate([0,20,-10-washer_m4_height()-nut_m4_height()])
    nut_m4();

  translate([0,20,-10-washer_m4_height()])
    washer_m4();
}

panel_vertical_axis(GAP);
