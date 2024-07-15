use <bolt_and_nut.scad>
use <assembly.scad>
use <big_bracket.scad>
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

bracket_pos_z = -200; // From top border

glue_radius=3;
glue_corner_radius=20;
glue_linear_side_length=70;

module mirror_glue() {
  rotate([90,0,0])
    for (a=[0:90:270]) {
      rotate([0,0,a]) {
          translate([glue_linear_side_length/2, glue_linear_side_length/2, 0])
            rotate_extrude(angle=90)
                translate([glue_corner_radius, 0]) circle(glue_radius);

          translate([glue_linear_side_length/2+glue_corner_radius, glue_linear_side_length/2, 0])
            rotate([90, 0, 0])
                cylinder(r=glue_radius, h=glue_linear_side_length);
      }
    }
}

module mirror(exploded) {
  translate([0,-mirror_depth-(exploded?8*GAP:0),0]) {
    translate([-mirror_width/2,0,-mirror_height/2])
      cube([mirror_width,mirror_depth,mirror_height]);

    // Add fake mirror image lines
    translate([-mirror_width/4,0,-mirror_height/5])
      rotate([0,45,0])
        cylinder(mirror_width/1.5, r=0.5, $fn=4);
    translate([-mirror_width/3,0,mirror_height/8])
      rotate([0,45,0])
        cylinder(mirror_width/4, r=0.5, $fn=4);
    translate([mirror_width/3,0,-mirror_height/8])
      rotate([0,225,0])
        cylinder(mirror_width/4, r=0.5, $fn=4);
  }

  if(exploded) {
    for (a=[0:90:270]) {
      rotate([0,a,0]) {
        translate([mirror_holder_width/2,0,mirror_holder_height/2])
          rotate([90,0,0])
            cylinder(8*GAP, r=exploded_line_radius());
      }
    }
  }
}

module mirror_holder(exploded, angle_x, angle_z) {
  translate([0,mirror_holder_pos_y-mirror_holder_depth,mirror_holder_middle_bolt_z]) {
    rotate([90,0,0])
      bolt_assembly_m4(
        bolt_length=60,
        bolt_gap_z=6*GAP,
        assembly_depth=mirror_holder_depth-mirror_holder_pos_y+panel_board_depth,
        washer_gap_z=3*GAP,
        exploded=exploded)
        countersunk_bolt_m4(60);

    translate([0,-(exploded?4*GAP:0),0]) {
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

        translate([0,0,-mirror_holder_middle_bolt_z]) {
          mirror_glue();
          translate([0,-glue_radius+1,0])
            mirror(exploded);
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
                impact_nut_assembly_m4(30, bolt_gap_z=5*GAP, assembly_depth=panel_board_depth+20,exploded=exploded)
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

  translate([0,panel_board_depth+(exploded?9*GAP:0),panel_board_height/2+bracket_pos_z]){
    big_bracket();

    for (hole_t=big_bracket_origin_to_vertical_holes_t()) {
      translate(hole_t+[0,big_bracket_depth()+2,0])
        rotate([-90,0,0])
          simple_assembly(15,gap=3*GAP,extra_line_length=9*GAP,exploded=exploded)
            wood_screw_d4(15);
    }
  }
}

panel_board(EXPLODED);
