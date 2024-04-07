use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>

$fa = 3;
$fs = 0.4;

LENGTH = 200;

function origin_to_hinges_t() = [[0,45,square_tube_width()],[0,LENGTH-45,square_tube_width()]];

GAP = 20;

exploded = true;

module panel_vertical_axis(gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(LENGTH);
    // Holes for left hinge
    for (hinge_t=origin_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+2,2,2);
      }
    }

    // Hole for rounded head bolt
    translate([square_tube_width()+1,LENGTH/2,square_tube_width()/2])
      rotate([0,-90,0])
        cylinder(square_tube_width()+2,2,2);
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate([0,0,exploded?gap:0])
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
        translate([0,0,-square_tube_width()-hinge_depth()-2*gap])
          cylinder(hinge_depth()+square_tube_width()+40+4*gap, 0.1, 0.1);
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate(hinge_t)
    for (hole_t=hinge_origin_to_holes_t()) {
      translate(hole_t)
        hinge_bolt_assembly();
    }
  }

  translate([square_tube_width(),LENGTH/2,square_tube_width()/2]) {
    rotate([0,90,0]) {
      translate([0,0,(exploded?30+gap:0)+1])
        round_head_bolt_m4(30);

      translate([0,0,+(exploded?-gap:0)-square_tube_width()-washer_m4_height()])
        washer_m4();

      translate([0,0,(exploded?-2*gap:0)-square_tube_width()-washer_m4_height()-nut_m4_height()])
        nut_m4();

      if(exploded) {
        color([0.8,0.8,0.8])
          translate([0,0,-square_tube_width()-hinge_depth()-2*gap])
            cylinder(hinge_depth()+square_tube_width()+40+4*gap, 0.1, 0.1);
      }
    }
  }
}

panel_vertical_axis(GAP);
