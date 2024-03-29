use <bolt_and_nut.scad>
use <square_tube.scad>
use <left_hinge.scad>

bottom_hinge_pos = 100;
top_hinge_pos = 300;

GAP = 10;

module pannel_vertical_axis(gap=0) {
  // square tube with all required holes
  difference() {
    square_tube(400);
    for (y=[-hinge_holes_gap():hinge_holes_gap():hinge_holes_gap()]) {
      translate([hinge_holes_offset(),bottom_hinge_pos+y,-1])
        cylinder(square_tube_width()+2,2,2);
      translate([hinge_holes_offset(),top_hinge_pos+y,-1])
        cylinder(square_tube_width()+2,2,2);
    }
  }

  translate([-hinge_platform_offset(),bottom_hinge_pos,square_tube_width()+hinge_depth() ])
    left_hinge_female();

  translate([-hinge_platform_offset(),top_hinge_pos,square_tube_width()+hinge_depth() ])
    left_hinge_female();

  for (y=[-hinge_holes_gap():hinge_holes_gap():hinge_holes_gap()]) {
    translate([hinge_holes_offset(),bottom_hinge_pos+y,square_tube_width()+hinge_depth()+2])
      countersunk_bolt_m4(40);
    translate([hinge_holes_offset(),top_hinge_pos+y,square_tube_width()+hinge_depth()+2])
      countersunk_bolt_m4(40);
  }

  translate([0,20,0])
    %round_head_bolt_m4(30);

  translate([0,20,-10-washer_m4_height()-nut_m4_height()])
    nut_m4();

  translate([0,20,-10-washer_m4_height()])
    washer_m4();
}

pannel_vertical_axis();
