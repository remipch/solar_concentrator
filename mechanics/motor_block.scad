use <motor.scad>
use <small_bracket.scad>

$fa = 5;
$fs = 0.4;

GAP = 20;

EXPLODED = true;

EXPLODED_LINE_RADIUS = 0.3; // if exploded=true, pices alignment are shown with "cylinder" lines of this radius


locking_ring_length = 6;
locking_ring_bolt_hole_radius = 1.5;

motor_axis_cylinder_length = 70;
motor_axis_cylinder_inner_radius = 5;
motor_axis_cylinder_outer_radius = 6;

axis_extension_length = 20;

motor_support_hole_offset = 5;
motor_support_hole_radius = 2;
motor_support_width = motor_bracket_length() + 4*motor_support_hole_offset;
motor_support_height = motor_bracket_length();
motor_support_depth = abs(small_bracket_origin_to_holes_t()[1].y) - motor_axis_offset();
motor_support_holes_t = [
  [motor_support_hole_offset-motor_support_width/2,motor_axis_offset(),motor_support_height/2],
  [motor_support_width/2-motor_support_hole_offset,motor_axis_offset(),motor_support_height/2]
];

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

module motor_support() {
  difference() {
    translate([-motor_support_width/2,motor_axis_offset(),0])
      cube([motor_support_width,motor_support_depth,motor_support_height]);
    for (hole_t=motor_support_holes_t) {
      translate(hole_t + [0,-1,0])
        rotate([-90,0,0])
          %cylinder(motor_support_depth+2,r=motor_support_hole_radius);
    }
  }

  // temp to see alignment
  translate([0,motor_axis_offset()+motor_support_depth,-15])
    #small_bracket();
}


module motor_block(exploded=false, gap=0) {
  motor_support();

  %motor();

  translate([0,0,-8-(exploded?gap:0)]) {
    locking_ring();

    translate([0,0,locking_ring_length-motor_axis_cylinder_length-(exploded?gap:0)]) {
      motor_axis_cylinder();

      translate([0,0,-(exploded?gap:0)]) {
        locking_ring();

        translate([0,0,-axis_extension_length/2+(exploded?locking_ring_length-gap-axis_extension_length/2:0)]) {
          cylinder(axis_extension_length,r=motor_axis_radius());

          if(exploded){
            cylinder(axis_extension_length+locking_ring_length+motor_axis_cylinder_length+4*gap,r=EXPLODED_LINE_RADIUS);
          }
        }
      }
    }
  }
}

motor_block(EXPLODED, GAP);
