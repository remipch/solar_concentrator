// en: square tube
// fr: tube carré
// Bricorama 25004802
// https://www.bricorama.fr/p/tube-carre-pour-m20-23.5-aluminium-brut-2.5m/4001116251900

function square_tube_width() = 23.5;

function square_tube_depth() = 1.5;

module square_tube(length) {
  inner_width = square_tube_width()-2*square_tube_depth();

  difference() {
    cube([square_tube_width(), length, square_tube_width()]);

    translate([square_tube_depth(), -1, square_tube_depth()])
      cube([inner_width, length+2, inner_width]);
  }
}

square_tube(100);
