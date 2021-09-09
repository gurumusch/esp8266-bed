difference() {
    union() {
        cube([22, 16.6, 18]);

        // first cable holder
        translate([22, 0, 0]) {
            cube([2.2, 2, 18]);
            translate([1.2, 0, 0]) {
                cube([1, 17, 18]);
            };
            translate([1.2, 16.5, 0]) {
                rotate([0, 0, -90]) {
                    cylinder($fn=10, h=18, r1=0.5, r2=0.5);
                }
            };
        };

        // second cable holder
        translate([24.2, 0, 0]) {
            cube([2.2, 2, 18]);
            translate([1.2, 0, 0]) {
                cube([1, 17, 18]);
            };
            translate([1.2, 16.5, 0]) {
                rotate([0, 0, -90]) {
                    cylinder($fn=10, h=18, r1=0.5, r2=0.5);
                }
            };
        };
    };
    union() {
        translate([-1,2,-1]) {
            cube([20, 13.6, 32]);
        };
        translate([5, -1, 5]) {
            cube([10, 4, 8]);
        };
    }
}