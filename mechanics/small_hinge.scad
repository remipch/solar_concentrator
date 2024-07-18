// en: small hinge
// fr: petite charnière

$fa = 10;
$fs = 0.1;

ANGLE = 130;

small_hinge_width = 35; // total width : both halves openned

function small_hinge_height() = 60;

function small_hinge_depth() = 1;

small_hinge_cylinder_count = 5; // for both halves

small_hinge_cylinder_radius = 2.5;

small_hinge_axis_radius = 1.5;

small_hinge_cylinder_stride = small_hinge_height() / small_hinge_cylinder_count;

small_hinge_cylinder_gap = 1;

small_hinge_cylinder_height = small_hinge_cylinder_stride - small_hinge_cylinder_gap;

function small_hinge_origin_to_holes_t() = [[8,-22,0],[8,0,0],[8,22,0]];

module half(even_cylinders) {
  difference() {
    union() {
      for(i=[0:small_hinge_cylinder_count-1]) {
        if( (even_cylinders && (i%2)==0) || (!even_cylinders && (i%2)==1)) {
          y = -small_hinge_height()/2 + i*small_hinge_cylinder_stride + small_hinge_cylinder_gap/2;
          translate([-small_hinge_cylinder_radius,y,small_hinge_cylinder_radius])
            rotate([-90,0,0])
              cylinder(small_hinge_cylinder_height,small_hinge_cylinder_radius,small_hinge_cylinder_radius);
          translate([-small_hinge_cylinder_radius,y,0])
            cube([small_hinge_cylinder_radius+1,small_hinge_cylinder_height,small_hinge_depth()]);
        }
      }
      translate([0,-small_hinge_height()/2,0])
        cube([small_hinge_width/2-small_hinge_cylinder_radius,small_hinge_height(),small_hinge_depth()]);
    }

    for (hole_t=small_hinge_origin_to_holes_t()) {
      translate(hole_t + [0,0,small_hinge_depth()+1])
        rotate([180,0,0])
          cylinder(small_hinge_depth()+2,4,1.5);
    }
  }
}

module small_hinge_move_to_other_half_origin(angle) {
  function small_hinge_second_half_origin_t() = [-2*small_hinge_cylinder_radius, 0, 2*small_hinge_cylinder_radius];

  translate([-small_hinge_cylinder_radius, 0, small_hinge_cylinder_radius])
    rotate([0,90-angle,0])
      translate([-small_hinge_cylinder_radius, 0, small_hinge_cylinder_radius])
        rotate([0,0,180])
          children();
}

module small_hinge(angle) {
  half(false);
  small_hinge_move_to_other_half_origin(angle)
    rotate([0,0,180])
      rotate([180,0,0])
        rotate([0,90,0])
          half(true);

  translate([-small_hinge_cylinder_radius,-small_hinge_height()/2,small_hinge_cylinder_radius])
    rotate([-90,0,0])
      cylinder(small_hinge_height(),small_hinge_axis_radius,small_hinge_axis_radius);
}

small_hinge(ANGLE);
%cube(10);

small_hinge_move_to_other_half_origin(ANGLE)
  %cube(10);
