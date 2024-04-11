
$fa = 10;
$fs = 0.1;

function small_bracket_depth() = 2;

function small_bracket_width() = 15;

function small_bracket_length() = 42;

function small_bracket_hole_offsets() = [15,35];

function small_bracket_origin_to_horizontal_holes_t() = [[0,-15,0],[0,-35,0]];

function small_bracket_origin_to_vertical_holes_t() = [for(z = small_bracket_hole_offsets()) [0,0,z]];

module small_bracket_half() {
  difference() {
    linear_extrude(height=small_bracket_depth()) {
      translate([-small_bracket_width()/2, - small_bracket_length() + small_bracket_width()/2])
        square([small_bracket_width(),small_bracket_length()-small_bracket_width()/2]);
      translate([0, -small_bracket_length()+small_bracket_width()/2])
        #circle(small_bracket_width()/2);
    }
    for (hole_t=small_bracket_origin_to_horizontal_holes_t()) {
      translate(hole_t + [0,0,-1])
        cylinder(small_bracket_depth()+2,1.8,4.5);
    }
  }
}

module small_bracket() {
  small_bracket_half();
  mirror([0,1,1])
    small_bracket_half();
}

small_bracket();
