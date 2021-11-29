union() {
    difference() {
        union() {
            cube([22, 43, 10]);
            translate([22, 0, 0]) {
                cube([2.2, 2, 10]);
                translate([1.2, 0, 0]) {
                    cube([2, 17, 10]);
                };
                translate([1.2, 16.5, 0]) {
                    rotate([0,0,-90]) {
                        cylinder($fn=10, h=10, r1=0.5, r2=0.5);
                    }
                };
                translate([0, 41, 0]) {
                    cube([2.2, 2, 10]);
                };
                translate([1.2, 26, 0]) {
                    cube([2, 17, 10]);
                };
                translate([1.2, 26.5, 0]) {
                    rotate([0,0,-90]) {
                        cylinder($fn=10, h=10, r1=0.5, r2=0.5);
                    }
                };
            };
        };
        union() {
            translate([-1,1,-1]) {
                cube([21, 41, 12]);
            };
        }
    };
    translate([-2, 0, 0]) {
        cube([2, 5, 10]);
    };
    translate([-1, 5, 0]) {
        rotate([0,0,-90]) {
            cylinder($fn=10, h=10, r1=1, r2=1);
        }
    };
    translate([-2, 38, 0]) {
        cube([2, 5, 10]);
    };
    translate([-1, 38, 0]) {
        rotate([0,0,-90]) {
            cylinder($fn=10, h=10, r1=1, r2=1);
        }
    };
}