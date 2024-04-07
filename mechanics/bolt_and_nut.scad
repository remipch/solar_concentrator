
$fa = 3;
$fs = 0.4;

// en: countersunk bolt
// fr: boulon tête fraisée
module countersunk_bolt(length, diameter, head_length, head_diameter) {
  difference() {
    rotate([180,0,0]) {
      intersection() {
        union() {
          cylinder(head_length,(head_diameter+1)/2,diameter/2);
          cylinder(length,diameter/2,diameter/2);
        }
        cylinder(length+head_length+1,head_diameter/2,head_diameter/2);
      }
    }
    translate([-head_diameter/4,-head_diameter/16,-2])
      cube([head_diameter/2,head_diameter/8,3]);
    translate([-head_diameter/16,-head_diameter/4,-2])
      cube([head_diameter/8,head_diameter/2,3]);
  }
}

module countersunk_bolt_m4(length) {
  color([0.5,0.5,0.5])
    countersunk_bolt(length,3.8,2.5,8);
}

module countersunk_bolt_m6(length) {
  color([0.5,0.5,0.5])
    countersunk_bolt(length,5.7,3.7,12);
}

// en: round head bolt
// fr: boulon poêlier
module round_head_bolt_m4(length) {
  color([0.5,0.5,0.5]) {
    difference() {
      union() {
        intersection() {
          translate([0,0,-4])
            sphere(6);
          translate([-10,-10,-0.1])
            cube(20);
        }
        d = 3.8;
        rotate([180,0,0])
          cylinder(length,d/2,d/2);
      }
      translate([-10,-0.5,1])
        cube([20,1,3]);
      translate([-0.5,-10,1])
        cube([1,20,3]);
    }
  }
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
  color([0.5,0.5,0.5])
    nut(nut_m4_height(),4,8);
}

function nut_m6_height() = 5;

module nut_m6() {
  color([0.5,0.5,0.5])
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
  color([0.5,0.5,0.5])
    washer(washer_m4_height(),5,10);
}

function washer_m6_height() = 1;

module washer_m6() {
  color([0.5,0.5,0.5])
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

translate([0,0,-10-washer_m4_height()-nut_m4_height()])
  %nut_m4();

translate([0,0,-10-washer_m4_height()])
  %washer_m4();

