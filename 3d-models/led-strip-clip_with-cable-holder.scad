module clip(
    cutout_dimensions,
    bottom_side_border,
    bottom_back_border,
    bottom_front_border,
    bottom_thickness,
    top_thickness,
    back_thickness,
    num_cable_holder) {
    
    module cable_holder(offset) {
        translate([offset, 0, 0]) {
            cube([2.2, cutout_dimensions[1], 2]);
            translate([1.2, 0, 0]) {
                cube([1, cutout_dimensions[1], 17]);
            };
            translate([1.2, 0, 16.5]) {
                rotate([-90, 0, 0]) {
                    cylinder($fn=10, h=cutout_dimensions[1], r1=0.5, r2=0.5);
                }
            };
        };
    }
    
    difference() {
        union() {
            cube([
                cutout_dimensions[0] + back_thickness,
                cutout_dimensions[1],
                cutout_dimensions[2] + bottom_thickness + top_thickness]);

            // cable holders
            for (x = [0:num_cable_holder - 1])
                cable_holder(cutout_dimensions[0] + back_thickness + (x * 2.2));

        };
        union() {
            translate([-1, -1, bottom_thickness]) {
                cube([
                    cutout_dimensions[0] + 1,
                    cutout_dimensions[1] + 2,
                    cutout_dimensions[2]]);
            };
            translate([bottom_front_border, bottom_side_border, -1]) {
                cube([
                    cutout_dimensions[0] - bottom_front_border - bottom_back_border,
                    cutout_dimensions[1] - (2 * bottom_side_border),
                    bottom_thickness + 2]);
            };
        }
    }
}

clip(
    /* cutout_dimensions    */ [19, 18, 13.6],
    /* bottom_side_border   */ 5,
    /* bottom_back_border   */ 4,
    /* bottom_front_border  */ 5,
    /* bottom_thickness     */ 2,
    /* top_thickness        */ 1,
    /* back_thickness       */ 3,
    /* num_cable_holder     */ 2
);