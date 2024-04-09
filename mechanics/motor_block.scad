use <motor.scad>

$fa = 5;
$fs = 0.4;

GAP = 20;

EXPLODED = true;

// show vertically (does not change the exported module orientation)
SHOWN_ROTATED = false;

LINE_RADIUS = 0.01; // if exploded=true, pices alignment are shown with "cylinder" lines of this radius


locking_ring_length = 6;
locking_ring_bolt_hole_radius = 1.5;

motor_axis_cylinder_length = 70;
motor_axis_cylinder_inner_radius = 5;
motor_axis_cylinder_outer_radius = 6;

function motor_axis_cylinder_origin_to_holes_t() = [[0,-motor_axis_cylinder_outer_radius,10],[0,-motor_axis_cylinder_outer_radius,motor_axis_cylinder_length-10]];

module locking_ring() {
  difference() {
    cylinder(locking_ring_length,r=motor_axis_cylinder_inner_radius);

    translate([0,0,-1])
      cylinder(locking_ring_length+2,r=motor_axis_radius());

    translate([0,0,locking_ring_length/2])
    rotate([90,0,0])
      cylinder(motor_axis_cylinder_inner_radius+1,r=locking_ring_bolt_hole_radius);
  }
}

module motor_axis_cylinder() {
  difference() {
    cylinder(motor_axis_cylinder_length,r=motor_axis_cylinder_outer_radius);

    translate([0,0,-1])
      cylinder(motor_axis_cylinder_length+2,r=motor_axis_cylinder_inner_radius);

    for (hole_t=motor_axis_cylinder_origin_to_holes_t()) {
      translate(hole_t + [0,0,-1])
        rotate([-90,0,0])
          cylinder(motor_axis_cylinder_outer_radius+1,r=locking_ring_bolt_hole_radius);
    }
  }

}

module motor_block(exploded=false, gap=0) {
  motor();

  translate([0,0,-8])
    locking_ring();

  translate([50,0,0])
    motor_axis_cylinder();

}

motor_block(EXPLODED, GAP);
