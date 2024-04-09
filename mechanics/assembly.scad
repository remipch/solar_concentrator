use <bolt_and_nut.scad>

DEFAULT_GAP = 20;

function exploded_line_radius() = 0.01;

// children(0) is screw (or other child element)
module simple_assembly(
  child_length,
  gap=DEFAULT_GAP,
  extra_line_length=0,
  exploded=false) {

  translate([0,0,exploded?child_length+gap:0]) {
    children(0);
    if(exploded) {
      rotate([180,0,0])
      cylinder(child_length+gap+extra_line_length, r=exploded_line_radius());
    }
  }
}

// children(0) is bolt
// children(1) is washer
// children(2) is nut
module bolt_assembly(
  bolt_length,
  bolt_gap_z,
  assembly_depth,
  washer_gap_y,
  washer_gap_z,
  washer_height,
  nut_gap_z,
  exploded=false) {

  translate([0,0,exploded?bolt_length+bolt_gap_z:0]) {
    children(0);
    if(exploded) {
      rotate([180,0,0])
      cylinder(bolt_length+bolt_gap_z+assembly_depth, r=exploded_line_radius());
    }
  }
  rotate([180,0,0]) {
    translate([0,0,assembly_depth]) {
      if(exploded) {
        line_angle = atan(washer_gap_y/washer_gap_z);
        line_length = sqrt(washer_gap_y^2 + washer_gap_z^2);
        rotate([line_angle,0,0])
        cylinder(line_length, r=exploded_line_radius());
      }
      translate([0,exploded?-washer_gap_y:0,exploded?washer_gap_z:0]) {
        children(1);
        translate([0,0,washer_height]) {
          if(exploded) {
            cylinder(nut_gap_z, r=exploded_line_radius());
          }
          translate([0,0,(exploded?nut_gap_z:0)]) {
              children(2);
          }
        }
      }
    }
  }
}

// children(0) is bolt
module bolt_assembly_m4(
  bolt_length,
  bolt_gap_z=DEFAULT_GAP,
  assembly_depth,
  washer_gap_y=0,
  washer_gap_z=DEFAULT_GAP,
  nut_gap_z=DEFAULT_GAP,
  exploded=false) {

  bolt_assembly(
    bolt_length = bolt_length,
    bolt_gap_z = bolt_gap_z,
    assembly_depth = assembly_depth,
    washer_gap_y = washer_gap_y,
    washer_gap_z = washer_gap_z,
    washer_height = washer_m4_height(),
    nut_gap_z = nut_gap_z,
    exploded = exploded) {

    children(0);
    washer_m4();
    nut_m4();
  }
}

assembly_depth=10;

translate([-10,-10,-assembly_depth])
  %cube([140,20,assembly_depth]);

translate([00,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth)
    countersunk_bolt_m4(20);

translate([20,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth, exploded=true)
    countersunk_bolt_m4(20);

translate([40,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth, washer_gap_y=DEFAULT_GAP*3, exploded=true)
    countersunk_bolt_m4(20);

translate([60,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth)
    round_head_bolt_m4(20);

translate([80,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth, exploded=true)
    round_head_bolt_m4(20);

translate([100,0,0])
  bolt_assembly_m4(bolt_length=20, assembly_depth=assembly_depth, washer_gap_y=DEFAULT_GAP*3, exploded=true)
    round_head_bolt_m4(20);

translate([120,0,0])
  simple_assembly(30,exploded=true)
    wood_screw_d4(30);
