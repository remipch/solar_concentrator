// en: flat profile
// fr: profile plat

function flat_profile_width() = 20;

function flat_profile_depth() = 2;

module flat_profile(length) {
  cube([flat_profile_width(), length, flat_profile_depth()]);
}

// slightly bended flat profile used as diagonal bar between vertical and horizontal axes
module flat_profile_diagonal_bar_for_axes(length) {
  bending_offset = length/5;

  rotate([-90,0,0])
    linear_extrude(flat_profile_width())
      polygon([
        [0,0],
        [0,flat_profile_depth()],
        [bending_offset,flat_profile_depth()],
        [length-bending_offset,0],
        [length,0],
        [length,-flat_profile_depth()],
        [length-bending_offset,-flat_profile_depth()],
        [bending_offset,0],
        [0,0]
      ]);
}

flat_profile(200);

translate([50,0,0])
  flat_profile_diagonal_bar_for_axes(200);
