
module truncate_negative_x(size, sin_amplitude=10) {
  intersection() {
    children();

    z_values = [-size.z/2:1:size.z/2];
    points=concat(
      [[-size.z/2,-size.x/2]],
      [for (z = z_values) [z, sin_amplitude*sin(2*z)]],
      [[size.z/2,-size.x/2]]
    );
    translate([0,size.y/2,0])
    rotate([90,-90,0])
    linear_extrude(size.y)
      polygon(points);
  }
}
