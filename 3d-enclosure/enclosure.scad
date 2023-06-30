// V 1.2 Correction / hint from molotok3D, some minor fixes
// V 1.1- added opening helper and an optional separating wall

wi=42;	// inner width, length & heigth
li=53;
th=2;	// wall thickness
h=8+th+12;
r=2;	// radius of rounded corners
opening_help=false;	// make a gap to ease opening of the cover, f.ex.
		// with a coin - girls are afraid of their finger nails ;-)

e=0.01;
ri=(r>th)?r-th:e;	// needed for the cover - needs to be larger than 0 for proper results
l=li-2*r;
w=wi-2*r;

// Custom sizes
battery_w = 34;
battery_h = 52;

board_w = 24;
board_h = 52;

bar_h = 8;
bar_r_off = 2;
pole_w = 1;

$fn=100; // Roundness

module box(){
	difference(){
		translate([0,0,-th])hull(){
			for (i=[[-w/2,-l/2],[-w/2,l/2],[w/2,-l/2],[w/2,l/2]]){
				translate(i)cylinder(r=r+th,h=h+th);
			}
		}
		hull(){
			for (i=[[-w/2,-l/2],[-w/2,l/2,],[w/2,-l/2],[w/2,l/2]]){
				translate(i)cylinder(r=r,h=h+0.1);
			}
		}
		translate([-w/2,l/2+r,h-2])rotate([0,90,0])cylinder(d=1.2,h=w);
		translate([-w/2,-l/2-r,h-2])rotate([0,90,0])cylinder(d=1.2,h=w);
		translate([w/2+r,l/2,h-2])rotate([90,0,0])cylinder(d=1.2,h=l);
		translate([-w/2-r,l/2,h-2])rotate([90,0,0])cylinder(d=1.2,h=l);

        usb_hole();
        
		// if you need some adjustment for the opening helper size or position,
		// this is the right place
		if (opening_help)translate([w/2-10,l/2+13.5,h-1.8])cylinder(d=20,h=10);
	}
}

module usb_hole() {
    translate([-0.5
    , -li/2-th-1, 11.5]) 
    linear_extrude(6) {
        square([12, th+2]);
    }
}

module bar(x,y,len){
    translate([x, y, 0])
    linear_extrude(bar_h) {
        square([th, len]);
    }
    
}

module box_bars() {
    // Battery right bar
    x = wi/2 - th - bar_r_off;
    bar(x, -li/2, li);

    // Battery left bar
    bar(x - battery_w - th, -li/2, li - 7);
}

module board_layer()  {
    difference() {
    
        translate([0,0,-th])hull(){
            for (i=[[-w/2,-l/2],[-w/2,l/2],[w/2,-l/2],[w/2,l/2]]){
                translate(i)cylinder(r=r,h=th);
            }
        }
    
        translate([-wi/2-e, li/2-6.9, -th-1]) linear_extrude(th+2) {
            square([4, 7]);
        }
    }
}

module board_layer_bars() {
    // Board right bar
    x = wi/2 - th - bar_r_off;
    bar(x, -li/2, li);

    // Board left bar
    bar(x - board_w - th, -th - 7.5, 36);
}

module pole(x, y) {
   translate([x, y, 0]) {
       cylinder(bar_h, pole_w, pole_w);
   }
}

module board_layer_poles() {
   x1 = wi/2 - th - pole_w - bar_r_off - 1.5;
   y1 = li/2 - th/2 - 2;
   pole(x1, y1);
   
   x2 = x1 - 19;
   pole(x2, y1);
    
   x3 = x1;
   y3 = y1 - 46;
   pole(x3, y3);
   
   x4 = x3 - 18;
   pole(x4, y3);
}

module cover(){
	difference(){
        translate([0,0,-th])hull(){
            for (i=[[-w/2,-l/2],[-w/2,l/2],[w/2,-l/2],[w/2,l/2]]){
                translate(i)cylinder(r=r+th,h=th);
            }
        }
        pin_holes();
    }
    
	difference(){
		translate([0,0,-th])hull(){
			for (i=[[-w/2,-l/2],[-w/2,l/2],[w/2,-l/2],[w/2,l/2]]){
				translate(i)cylinder(r=r,h=th+3);
			}
		}
		hull(){
			for (i=[[-w/2,-l/2],[-w/2,l/2],[w/2,-l/2],[w/2,l /2]]){
				if (r>th){
					translate(i)cylinder(r=r-th,h=3+e);
				}else{
					translate(i)cylinder(r=e,h=3+e);
				}
			}
		}
        pin_holes();
	}
	translate([-w/2+1,l/2+r-0.2,2])rotate([0,90,0])cylinder(d=1.2,h=w-2);
	translate([-w/2+1,-l/2-r+0.2,2])rotate([0,90,0])cylinder(d=1.2,h=w-2);
	translate([w/2+r-0.2,l/2-1,2])rotate([90,0,0])cylinder(d=1.2,h=l-2);
	translate([-w/2-r+0.2,l/2-1,2])rotate([90,0,0])cylinder(d=1.2,h=l-2);

}

// Holes for the pins
module pin_holes() {
    hole_w = 4;
    hole1_len = 42;
    hole2_len = 32;
    
    // Hole1
    x1 = wi/2 - bar_r_off - th - hole_w;
    y1 = -hole1_len/2 - 0.5;
    translate([x1,y1,-th*2]) linear_extrude(th*3) {
        square([hole_w, hole1_len]);
    }
    
    // Hole2
    x2 = x1 - 20;
    y2 = y1;
    translate([x2,y2,-th*2]) linear_extrude(th*3) {
        square([hole_w, hole2_len]);
    }
}

module print_part() {
    // Box
    box();
    box_bars();

    // Board layer
    translate([wi + th + 3, 0, 0]) {
    // translate([0, 0, th + bar_h]) {
        board_layer();
        board_layer_bars();
        board_layer_poles();
    }
    
    // Cover
    // translate([0,li+3+2*th,0]) cover(); // "real" position
    translate([-wi-3-2*th,0,0]) cover();
}

// For 2D print (Render, export to SVG and open with Inkscape)
// projection(cut = true) translate([0,0,-1]) print_part();
// projection(cut = true) translate([0,0,0]) print_part();

print_part();

