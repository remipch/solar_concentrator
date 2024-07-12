use <bolt_and_nut.scad>
use <assembly.scad>
use <pulley.scad> // Only for the "ring" part

$fa = 10;
$fs = 0.1;

GAP = 20;

EXPLODED = false;

mirror_width = 150;
mirror_depth = 4;
mirror_height = 150;

mirror_holder_width = 130;
mirror_holder_depth = 10;
mirror_holder_height = 130;
mirror_holder_pos_y = -20;
mirror_holder_middle_bolt_z = 15; // from mirror_holder center

mirror_holder_bolt_t = [ // from mirror_holder center
  [-55,0,55],
  [55,0,55],
  [0,0,-55]
];


mirror_columns = 4;
mirror_rows = 6;

mirror_period_x = mirror_width + 5;
mirror_period_z = mirror_height + 5;

panel_board_width = mirror_columns * mirror_period_x;
panel_board_depth = 15;
panel_board_bottom_margin = 30;
panel_board_height = mirror_rows * mirror_period_z;
panel_board_total_height = panel_board_height + panel_board_bottom_margin;

ring_hole_x = 20; // From bottom corners
ring_hole_z = 15; // From bottom corners

module mirror_holder(exploded, angle_x, angle_z) {
  translate([0,mirror_holder_pos_y-mirror_holder_depth,mirror_holder_middle_bolt_z]) {
    rotate([90,0,0])
      bolt_assembly_m4(
        bolt_length=60,
        bolt_gap_z=3*GAP,
        assembly_depth=mirror_holder_depth-mirror_holder_pos_y+panel_board_depth,
        washer_gap_z=2*GAP,
        exploded=exploded)
        countersunk_bolt_m4(60);

    translate([0,-(exploded?2*GAP:0),0]) {
      rotate([angle_x,0,angle_z]) {
        difference() {
          translate([-mirror_holder_width/2,0,-mirror_holder_height/2-mirror_holder_middle_bolt_z])
            cube([mirror_holder_width,mirror_holder_depth,mirror_holder_height]);

          translate([0,-1,0])
            rotate([-90,0,0]) {
              cylinder(mirror_holder_depth+2,r=2.5);
              cylinder(4,r1=6,r2=2.5);
            }
        }
      }
    }
  }
}

module panel_board(exploded=false, gap=GAP) {
  difference() {
    translate([-panel_board_width/2,0,-panel_board_height/2-panel_board_bottom_margin])
      cube([panel_board_width,panel_board_depth,panel_board_total_height]);

    for (c=[0:mirror_columns-1]) {
      x = mirror_period_x * (-(mirror_columns-1)/2 + c);
      for (r=[0:mirror_rows-1]) {
        z = mirror_period_z * (-(mirror_rows-1)/2 + r);
        translate([x,-1,z]) {
          translate([0,0,mirror_holder_middle_bolt_z])
            rotate([-90,0,0])
              cylinder(panel_board_depth+2,r=2.5);
          for (hole_t=mirror_holder_bolt_t) {
            translate(hole_t)
              rotate([-90,0,0])
                cylinder(panel_board_depth+2,r=3);
          }
        }
      }
    }

    for (x=[-panel_board_width/2+ring_hole_x,panel_board_width/2-ring_hole_x]) {
      translate([x,-1,-panel_board_height/2-panel_board_bottom_margin+ring_hole_z])
        rotate([-90,0,0])
          cylinder(panel_board_depth+2,r=3);
    }
  }

  for (c=[0:mirror_columns-1]) {
    x = mirror_period_x * (-(mirror_columns-1)/2 + c);
    angle_z = -x/60; // not the exact angle, just to show some mirror orientation
    for (r=[0:mirror_rows-1]) {
      z = mirror_period_z * (-(mirror_rows-1)/2 + r);
      angle_x = z/60; // not the exact angle, just to show some mirror orientation

      if(!exploded || (c==0&&r==1)){
        translate([x,0,z]){
          mirror_holder(exploded, angle_x, angle_z);

          for (hole_t=mirror_holder_bolt_t) {
            translate(hole_t+[0,panel_board_depth+20,0])
              rotate([-90,0,0])
                impact_nut_assembly_m4(30, bolt_gap_z=3*GAP, assembly_depth=panel_board_depth+20,exploded=exploded)
                  countersunk_bolt_m4(60);
          }
        }
      }
    }
  }

  for (x=[-panel_board_width/2+ring_hole_x,panel_board_width/2-ring_hole_x]) {
    translate([x,panel_board_depth,-panel_board_height/2-panel_board_bottom_margin+ring_hole_z])
      rotate([-90,0,0])
        bolt_assembly_m6(bolt_length=40,bolt_gap_z=3*GAP,assembly_depth=panel_board_depth,washer_gap_z=GAP,nut_gap_z=2*GAP,exploded=exploded)
          rotate([-90,0,0])
            fixed_ring(30);
  }
}

panel_board(EXPLODED);
