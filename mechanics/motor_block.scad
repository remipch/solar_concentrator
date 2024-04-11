use <motor.scad>
use <small_bracket.scad>
use <bolt_and_nut.scad>
use <assembly.scad>

$fa = 5;
$fs = 0.4;

GAP = 20;

EXPLODED = true;


locking_ring_length = 6;
locking_ring_bolt_hole_radius = 1.5;
locking_ring_bolt_length = 10;

motor_axis_cylinder_length = 70;
motor_axis_cylinder_inner_radius = 5;
motor_axis_cylinder_outer_radius = 6;

axis_extension_length = 20;

motor_support_hole_offset = 5;
motor_support_hole_radius = 2;
motor_support_width = motor_bracket_length() + 4*motor_support_hole_offset;
motor_support_height = motor_bracket_length();
function motor_support_depth() = abs(small_bracket_origin_to_holes_t()[1].y) - motor_axis_offset();
function motor_support_axis_offset() = motor_support_depth() + motor_axis_offset();

function motor_support_holes_t() = [
  [motor_support_hole_offset-motor_support_width/2,motor_axis_offset(),motor_support_height/2],
  [motor_support_width/2-motor_support_hole_offset,motor_axis_offset(),motor_support_height/2]
];
echo(motor_support_depth=motor_support_depth);

function motor_axis_cylinder_origin_to_holes_t() = [
  [0,-motor_axis_cylinder_outer_radius,locking_ring_length],
  [0,-motor_axis_cylinder_outer_radius,motor_axis_cylinder_length-locking_ring_length]
];

module locking_ring() {
  difference() {
    translate([0,0,-locking_ring_length/2])
      cylinder(locking_ring_length,r=motor_axis_cylinder_inner_radius);

    translate([0,0,-locking_ring_length/2 - 1])
      cylinder(locking_ring_length+2,r=motor_axis_radius());

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
      translate(hole_t + [0,-1,0])
        rotate([-90,0,0])
          cylinder(motor_axis_cylinder_outer_radius+1,r=locking_ring_bolt_hole_radius);
    }
  }
}

module motor_support() {
  difference() {
    translate([-motor_support_width/2,motor_axis_offset(),0])
      cube([motor_support_width,motor_support_depth(),motor_support_height]);
    for (hole_t=motor_support_holes_t()) {
      translate(hole_t + [0,-1,0])
        rotate([-90,0,0])
          cylinder(motor_support_depth()+2,r=motor_support_hole_radius);
    }
  }

}


module motor_block(exploded=false, gap=0) {
  motor_support();

  translate([0,-(exploded?gap:0),0]) {
    motor();

    for (hole_t=motor_bracket_holes_t()) {
      translate(hole_t + [0,-1,0])
        rotate([90,0,0])
          simple_assembly(15,exploded=exploded,gap=3*gap,extra_line_length=gap) {
            wood_screw_d3(15);
          }
    }

    translate([0,0,-motor_axis_length() + locking_ring_length/2 - (exploded?gap:0)]) {
      locking_ring();

      translate([0,0,locking_ring_length - motor_axis_cylinder_length - (exploded?gap:0)]) {
        motor_axis_cylinder();

        for (hole_t=motor_axis_cylinder_origin_to_holes_t()) {
          translate(hole_t + [0,motor_axis_cylinder_outer_radius-motor_axis_radius()-locking_ring_bolt_length,0])
            rotate([90,0,0])
              simple_assembly(locking_ring_bolt_length,exploded=exploded) {
                countersunk_bolt_m3(locking_ring_bolt_length);
              }
        }

        translate([0,0,locking_ring_length-(exploded?gap:0)]) {
          locking_ring();

          translate([0,0,locking_ring_length/2 - axis_extension_length - (exploded?gap:0)]) {
            cylinder(axis_extension_length,r=motor_axis_radius());

            if(exploded){
              cylinder(axis_extension_length+locking_ring_length+motor_axis_cylinder_length+4*gap,r=exploded_line_radius());
            }
          }
        }
      }
    }
  }
}

motor_block(EXPLODED, GAP);
