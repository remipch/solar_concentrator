// en: flat profile
// fr: profile plat

function flat_profile_width() = 20;

function flat_profile_depth() = 2;

module flat_profile(length) {
  cube([flat_profile_width(), length, flat_profile_depth()]);
}

flat_profile(200);
