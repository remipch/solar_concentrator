
$fa = 10;
$fs = 0.1;

function big_bracket_depth() = 3;
bracket_big_width = 30;
bracket_small_width = 15;

function big_bracket_origin_to_horizontal_holes_t() = [[6,85,0],[-3,235,0],[0,385,0]];
function big_bracket_origin_to_vertical_holes_t() = [[6,0,-45],[-3,0,-165],[0,0,-285]];

module big_bracket() {
  difference() {
    union() {
      translate([0,0,-big_bracket_depth()])
        linear_extrude(height=big_bracket_depth(),center=false)
          polygon([[-bracket_big_width/2,0],[bracket_big_width/2,0],[bracket_small_width/2,400],[-bracket_small_width/2,400]]);
      rotate([-90,0,0])
        linear_extrude(height=big_bracket_depth(),center=false)
          polygon([[-bracket_big_width/2,0],[bracket_big_width/2,0],[bracket_small_width/2,300],[-bracket_small_width/2,300]]);

      rotate([0,90,0])
        translate([0,0,-big_bracket_depth()/2])
          linear_extrude(height=big_bracket_depth(),center=false)
            polygon([[0,0],[150,0],[150,10],[40,20],[20,40],[10,150],[0,150]]);
    }
    for (hole_t=big_bracket_origin_to_horizontal_holes_t()) {
      translate(hole_t+[0,0,-big_bracket_depth()-1])
        cylinder(big_bracket_depth()+2,r=2.5);
    }
    for (hole_t=big_bracket_origin_to_vertical_holes_t()) {
      translate(hole_t+[0,-1,0])
        rotate([-90,0,0])
          cylinder(big_bracket_depth()+2,r=2.5);
    }
  }
}

big_bracket();
