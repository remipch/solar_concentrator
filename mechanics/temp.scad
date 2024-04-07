use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>

$fa = 3;
$fs = 0.4;

function origin_to_hinges_t() = [[0,100,square_tube_width()],[0,300,square_tube_width()]];

GAP = 20;

exploded = true;

module panel_vertical_axis(gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(400);
    for (hinge_t=origin_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+2,2,2);
      }
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate(hinge_t)
      left_hinge_female();
  }

  module hinge_bolt_assembly() {
    translate([0,0,hinge_depth()+(exploded?40+2*gap:0)+1])
      countersunk_bolt_m4(40);

    translate([0,0,+(exploded?-gap:0)-square_tube_width()-washer_m4_height()])
      washer_m4();

    translate([0,0,(exploded?-2*gap:0)-square_tube_width()-washer_m4_height()-nut_m4_height()])
      nut_m4();

    if(exploded) {
      color([0.8,0.8,0.8])
        translate([0,0,-2*gap])
          cube([0.01,0.01,40+4*gap]);
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate(hinge_t)
    for (hole_t=hinge_origin_to_holes_t()) {
      translate(hole_t)
        hinge_bolt_assembly();
    }
  }
}

panel_vertical_axis(GAP);
