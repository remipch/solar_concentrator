use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>

$fa = 3;
$fs = 0.4;

LENGTH = 200;

GAP = 20;

EXPLODED = true;

LINE_RADIUS = 0.01; // if exploded=true, pices alignment are shown with "cylinder" lines of this radius

function origin_to_hinges_t() = [[0,45,square_tube_width()],[0,LENGTH-45,square_tube_width()]];

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
    translate([0,0,EXPLODED?gap:0])
      translate(hinge_t)
        left_hinge_female();
  }

  module hinge_bolt_assembly(bolt_t) {
    short_bolt = (bolt_t[1]>LENGTH-20); // specific case for the last bolt : shorter because another bolt will be in the same axis
    bolt_length = short_bolt ? 15 : 40;
    bolt_gap = EXPLODED ? gap : 0;
    washer_gap = EXPLODED ? (short_bolt ? 0 : -gap) : 0;
    washer_z = washer_gap + (short_bolt ? -square_tube_depth()-washer_m4_height():-square_tube_width()-washer_m4_height());
    nut_gap = EXPLODED ? (short_bolt ? 0 : -2*gap) : 0;
    nut_z = nut_gap + (short_bolt ? -square_tube_depth()-washer_m4_height()-nut_m4_height(): -square_tube_width()-washer_m4_height()-nut_m4_height());
    translate(bolt_t) {
      translate([0,0,hinge_depth()+(EXPLODED?40+2*gap:0)+1])
        countersunk_bolt_m4(bolt_length);

      translate([0,0,washer_z])
        washer_m4();

      translate([0,0,nut_z])
        nut_m4();

      if(EXPLODED) {
        color([0.8,0.8,0.8])
          translate([0,0,-square_tube_width()-hinge_depth()-2*gap])
            cylinder(hinge_depth()+square_tube_width()+40+4*gap, r=LINE_RADIUS);
      }
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    for (hole_t=hinge_origin_to_holes_t()) {
        hinge_bolt_assembly(hinge_t+hole_t);
    }
  }

  translate([square_tube_width(),LENGTH/2,square_tube_width()/2]) {
    rotate([0,90,0]) {
      translate([0,0,(EXPLODED?30+gap:0)+1])
        round_head_bolt_m4(30);

      translate([0,0,+(EXPLODED?-gap:0)-square_tube_width()-washer_m4_height()])
        washer_m4();

      translate([0,0,(EXPLODED?-2*gap:0)-square_tube_width()-washer_m4_height()-nut_m4_height()])
        nut_m4();

      if(EXPLODED) {
        color([0.8,0.8,0.8])
          translate([0,0,-square_tube_width()-hinge_depth()-2*gap])
            cylinder(hinge_depth()+square_tube_width()+40+4*gap, r=LINE_RADIUS);
      }
    }
  }
}

panel_vertical_axis(GAP);
