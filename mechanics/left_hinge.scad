// en: left hinge
// fr: paumelle gauche
// 80x40 G
// Bricorama 530050

$fa = 3;
$fs = 0.4;

// gap between hole axes
function hinge_depth() = 2.5;

function hinge_origin_to_axis_t() = [-8,0,hinge_depth()];

function hinge_male_origin_to_axis_t() = [8,0,hinge_depth()];

function hinge_female_origin_to_axis_t() = hinge_origin_to_axis_t();

function hinge_origin_to_holes_t() = [[6,-32,0],[6,8,0],[6,30,0]];

module half() {
  translate(hinge_origin_to_axis_t()) {
    rotate([-90,0,0])
      translate([0,0,0.5])
        cylinder(29.5,5,5);

    translate([0,30,0])
      sphere(5);
  }

  difference() {
    linear_extrude(height=hinge_depth()) {
      translate([6, 35])
        circle(6);
      translate([6, -35])
        circle(6);
      translate([0,-35])
        square([12,70]);
      translate([-6,0.5])
        square([7,26]);
    }
    for (hole_t=hinge_origin_to_holes_t()) {
      translate(hole_t + [0,0,hinge_depth()+1])
        rotate([180,0,0])
          cylinder(hinge_depth()+2,4.5,1.8);
    }
  }
}

module left_hinge_female() {
  difference() {
    half();
    translate(hinge_origin_to_axis_t())
      rotate([-90,0,0])
        cylinder(20,3,3);
  }
}

module left_hinge_male() {
  rotate([0,0,180]) {
    half();
    translate(hinge_origin_to_axis_t()+[0,1,0])
      rotate([90,0,0])
        cylinder(20,3,3);
  }
}

left_hinge_female();

translate([0,-100,0])
  left_hinge_male();

translate([0,100,0]) {
  translate(-hinge_female_origin_to_axis_t())
    left_hinge_female();

  translate(-hinge_male_origin_to_axis_t())
    left_hinge_male();
}
