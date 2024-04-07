use <bolt_and_nut.scad>
use <square_tube.scad>
use <small_hinge.scad>

$fa = 3;
$fs = 0.4;

LENGTH = 450;

GAP = 20;

SMALL_HINGE_ANGLE = 60;

EXPLODED = true;

LINE_RADIUS = 0.01; // if exploded=true, pieces alignment are shown with "cylinder" lines of this radius

function origin_to_hinges_t() = [[0,small_hinge_height()/2,square_tube_width()],[0,LENGTH-small_hinge_height()/2,square_tube_width()]];

diagonal_fixpoint_offset = 100;

function origin_to_diagonal_fixpoint_t() = [square_tube_width(),LENGTH/2+diagonal_fixpoint_offset,square_tube_width()/2];

module panel_vertical_axis(gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(LENGTH);
    for (hinge_t=origin_to_hinges_t()) {
      translate(hinge_t)
      for (hole_t=small_hinge_origin_to_holes_t()) {
        translate(hole_t + [0,0,1])
          rotate([180,0,0])
            cylinder(square_tube_width()+2,2,2);
      }
    }
  }

  for (hinge_t=origin_to_hinges_t()) {
    translate([0,0,EXPLODED?gap:0])
      translate(hinge_t)
        small_hinge(SMALL_HINGE_ANGLE);
  }

  module hinge_bolt_assembly() {
    translate([0,0,small_hinge_depth()+(EXPLODED?40+2*gap:0)+1])
      countersunk_bolt_m4(40);

    translate([0,0,+(EXPLODED?-gap:0)-square_tube_width()-washer_m4_height()])
      washer_m4();

    translate([0,0,(EXPLODED?-2*gap:0)-square_tube_width()-washer_m4_height()-nut_m4_height()]) {
      nut_m4();

      if(EXPLODED)
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
}

module panel_board(gap=0) {
  translate([-200,-LENGTH,(EXPLODED?-gap:0)-10])
    %cube([400,LENGTH,10]);
}

panel_vertical_axis(GAP);

translate([0,0,square_tube_width()+(EXPLODED?GAP:0)])
  small_hinge_move_to_other_half_origin(SMALL_HINGE_ANGLE)
    panel_board(GAP);
