
$fa = 5;
$fs = 0.4;

function motor_radius() = 20;

bracket_depth = 2;
bracket_hole_offset = 5;
bracket_hole_radius = 2;
bracket_length = 60;

function motor_bracket_holes_t() = [
  [-bracket_length/2+bracket_hole_offset,motor_radius(),bracket_hole_offset],
  [-bracket_length/2+bracket_hole_offset,motor_radius(),bracket_length-bracket_hole_offset],
  [bracket_length/2-bracket_hole_offset,motor_radius(),bracket_hole_offset],
  [bracket_length/2-bracket_hole_offset,motor_radius(),bracket_length-bracket_hole_offset]
];

module motor() {
  cylinder(30,r=motor_radius());
  translate([0,0,29])
    cylinder(50,r=16);
  rotate([180,0,0])
    cylinder(10,r=3);
  translate([-motor_radius(),-motor_radius(),0])
    cube([motor_radius()*2,motor_radius()*2,bracket_depth]);

  difference() {
    translate([-bracket_length/2,motor_radius(),0])
      cube([bracket_length,bracket_depth,bracket_length]);

    for (hole_t=motor_bracket_holes_t()) {
      translate(hole_t + [0,-1,0])
        rotate([-90,0,0])
          cylinder(bracket_depth+2,r=bracket_hole_radius);
    }
  }
}

motor();
