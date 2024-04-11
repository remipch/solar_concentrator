use <bolt_and_nut.scad>

$fa = 3;
$fs = 0.2;

module ring(bolt_length) {
  translate([0,-20,0]) {
    translate([0,8,0])
      rotate([-90,0,0])
        cylinder(bolt_length+12,r=3);

    rotate_extrude()
      translate([8, 0, 0])
        circle(r = 2.5);
  }
}

module wheel() {
  rotate_extrude()
    polygon([[2,6],[11,6],[11,4],[9,4],[8,3],[8,-3],[9,-4],[11,-4],[11,-6],[2,-6],[2,6]]);
}

module wheel_holder() {
  rotate([0,90,180])
    ring(11);

  translate([0,-26,0])
    wheel();

  translate([-8,-10,-8])
    cube([16,1,16]);

  translate([-8,-35,-8])
    cube([16,25,1]);

  translate([-8,-35,7])
    cube([16,25,1]);

  translate([0,-26,-9])
    cylinder(18,r=4);
}

module fixed_ring(bolt_length) {
  ring(bolt_length);

  rotate([-90,0,0]){
    translate([0,0,-washer_m6_height()])
      washer_m6();
    translate([0,0,-washer_m6_height()-nut_m6_height()])
      nut_m6();
  }
}

module pulley(bolt_length) {
  fixed_ring(bolt_length);

  translate([0,-50,0])
    wheel_holder();
}

module vertical_pulley(bolt_length) {
  rotate([-90,0,0])
    pulley(bolt_length);
}

pulley(40);

