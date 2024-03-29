
$fa = 3;
$fs = 0.4;

// en: countersunk bolt
// fr: boulon tête fraisée
module countersunk_bolt(length, diameter, head_length, head_diameter) {
  rotate([180,0,0]) {
    cylinder(head_length,head_diameter/2,diameter/2);
    cylinder(length,diameter/2,diameter/2);
  }
}

module countersunk_bolt_m4(length) {
  countersunk_bolt(length,4,2.5,8);
}

module countersunk_bolt_m6(length) {
  countersunk_bolt(length,6,3.7,12);
}

// en: round head bolt
// fr: boulon poêlier
module round_head_bolt_m4(length) {
  intersection() {
    translate([0,0,-4])
      sphere(6);
    translate([-10,-10,0])
      cube(20);
  }
  d = 3.8;
  rotate([180,0,0])
    cylinder(length,d/2,d/2);
}

// en: nut
// fr: écrou
module nut(height, hole_diameter, external_diameter) {
  difference() {
    cylinder(height, external_diameter/2, external_diameter/2, $fn = 6);

    translate([0,0,-1])
      cylinder(height+2, hole_diameter/2, hole_diameter/2);
  }
}

function nut_m4_height() = 3;

module nut_m4() {
  nut(nut_m4_height(),4,8);
}

function nut_m6_height() = 5;

module nut_m6() {
  nut(nut_m6_height(),6,11.3);
}

// en: washer
// fr: rondelle
module washer(height, inner_diameter, outer_diameter) {
  difference() {
    cylinder(height, outer_diameter/2, outer_diameter/2);

    translate([0,0,-1])
      cylinder(height+2, inner_diameter/2, inner_diameter/2);
  }
}

function washer_m4_height() = 0.8;

module washer_m4() {
  washer(washer_m4_height(),5,10);
}

function washer_m6_height() = 1;

module washer_m6() {
  washer(washer_m6_height(),6.2,18);
}

countersunk_bolt_m4(30);

translate([20,0,0])
  countersunk_bolt_m6(30);

translate([0,20,0])
  round_head_bolt_m4(30);

translate([0,40,0])
  nut_m4();

translate([20,40,0])
  nut_m6();

translate([0,60,0])
  washer_m4();

translate([20,60,0])
  washer_m6();


