// en: left hinge
// fr: paumelle gauche
// 80x40 G
// Bricorama 530050

$fa = 3;
$fs = 0.4;

module half() {
  rotate([90,0,0])
    translate([0,0,0.5])
      cylinder(29.5,5,5);

  translate([0,-30,0])
    sphere(5);

  difference() {
    linear_extrude(height=2.5) {
      translate([14, 35])
        circle(6);
      translate([14, -35])
        circle(6);
      translate([8,-35])
        square([12,70]);
      translate([1,-26.5])
        square([15,26]);
    }
    for (y=[-35:35:35])
      translate([14,y,-5])
        cylinder(10,2.5,2.5);
  }
}

module left_hinge_male() {
  half();
  rotate([-90,0,0])
    translate([0,0,-0.5])
      cylinder(20,3,3);
}

module left_hinge_female() {
  rotate([180,0,0]) {
    difference() {
      half();
      rotate([90,0,0])
        translate([0,0,-1])
          cylinder(20,3,3);
    }
  }
}


left_hinge_male();

left_hinge_female();

translate([-40,0,0])
  left_hinge_male();

translate([40,0,0])
  left_hinge_female();
