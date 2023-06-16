// Which one would you like to see?
part = "both"; // [box:Box only, top: Top cover only, both: Box and top cover]

// Size of your printer's nozzle in mm
nozzle_size = 0.4;

// Number of walls the print should have
number_of_walls = 4; // [1:5]

// Tolerance (use 0.2 for FDM)
tolerance = 0.2; // [0.1:0.1:0.4]

// Outer x dimension in mm
x=70+2*number_of_walls*nozzle_size+2*3*nozzle_size;

// Outer y dimension in mm
y=51+2*number_of_walls*nozzle_size+2*3*nozzle_size;

// Outer z dimension in mm
z=18+2*number_of_walls*nozzle_size+2*3*nozzle_size;

// Radius for rounded corners in mm
radius=4; // [1:20]

/* Hidden */
$fn=100;

wall_thickness=nozzle_size*number_of_walls;
hook_thickness = 3*nozzle_size;

top_cover_wall_thickness = hook_thickness + wall_thickness;

// Custom additions
board_w = 24;
board_h = 53;

battery_w = 35;
battery_h = 53;

bar_w = wall_thickness;
bar_h = 8;
bar1_off = 4 - bar_w;
bar2_len = 33;
bar3_off = 10;
bar3_len = 20;

pole_z = 4;
pole_w = 1;

usb_w = 12;
usb_off = 0;
usb_z = 6;

module bottom_box () {
    difference(){
        // Solid box
        linear_extrude(z-wall_thickness){
            minkowski(){
                square([x-radius*2,y-radius*2], center=true);
                circle(radius, center=true);
            }
        }
        
        // Hollow out
        translate([0,0,wall_thickness]) linear_extrude(z){
            minkowski(){
                square([x-radius*2-wall_thickness*2+wall_thickness*2,y-radius*2-wall_thickness*2+wall_thickness*2], center=true);
                circle(radius-wall_thickness);
            }
        }
        
        // Cut out USB outlet
        usb_outlet();
    }
    left_hook(); // left hook
    rotate([180,180,0]) left_hook(); // right hook
    front_hook(); // front hook
    rotate([180,180,0]) front_hook(); // back hook
    // TODO: hooks on the other two sides
    
    bottom_bars();
    bottom_poles();
}

module usb_outlet() {
    translate([(x/2) - wall_thickness - bar1_off - bar_w - usb_w/2 - 6.5, 
     -(y/2) + (wall_thickness/2), 
     wall_thickness + usb_off]) 
    linear_extrude(usb_z) {
        square([usb_w, wall_thickness+1], center=true);
    }
}

module bar(x,y,h) {
    translate([x, y, wall_thickness])
    linear_extrude(bar_h) {
        square([bar_w, h], center=true);
    }    
}

module bottom_bars() {
   // Bar 1
   x1 = (x/2) - wall_thickness - bar1_off - (bar_w/2);
   y1 = 0;
   bar(x1, y1, y - (wall_thickness*2));

   // Bar 2
   x2 = x1 - board_w - bar_w;
   y2 = y/2 - wall_thickness - bar2_len/2;
   bar(x2, y2, bar2_len);

   // Bar 3
   x3 = x2 - battery_w - bar_w;
   y3 = y/2 - wall_thickness - bar3_len/2 - bar3_off;
   bar(x3, y3, bar3_len);
}

module pole(x, y) {
   translate([x, y, pole_z/2 + wall_thickness]) {
       cylinder(pole_z, pole_w, center=true);
   }
}


module bottom_poles() {
   x1 = (x/2) - wall_thickness - bar1_off - bar_w - pole_w - 1.5;
   y1 = (y/2) - wall_thickness - 3.5;
   pole(x1, y1);
   
   x2 = x1 - 19;
   pole(x2, y1);
    
   x3 = (x/2) - wall_thickness - bar1_off - bar_w - pole_w - 2.5;
   y3  = (y/2) - wall_thickness - 49.5;
   pole(x3, y3);
   
   x4 = x3 - 18;
   pole(x4, y3);
   
}

module left_hook () {
    
    translate([(x-2*wall_thickness)/2,-y/2+radius*2,z-wall_thickness]) rotate([0,90,90]) {
        difference(){
            linear_extrude(y-2*radius*2){
    polygon(points=[[0,0],[2*hook_thickness,0],[hook_thickness,hook_thickness]], center=true);
        }
             translate([hook_thickness, hook_thickness, 0]) rotate([45,0,0]) cube(2*hook_thickness, center=true);
             translate([hook_thickness, hook_thickness, y-2*radius*2]) rotate([45,0,0]) cube(2*hook_thickness, center=true);        
        }
    }
}
    

module front_hook () {
    translate([(-x+4*radius)/2,-y/2+wall_thickness,z-wall_thickness]) rotate([90,90,90]) {
        difference(){
        linear_extrude(x-2*radius*2){
    polygon(points=[[0,0],[2*hook_thickness,0],[hook_thickness,hook_thickness]], center=true);
    }
             translate([hook_thickness, hook_thickness, 0]) rotate([45,0,0]) cube(2*hook_thickness, center=true);
             translate([hook_thickness, hook_thickness, x-2*radius*2]) rotate([45,0,0]) cube(2*hook_thickness, center=true);
        }
    }
}


module right_grove () {
    translate([-tolerance/2+(x-2*wall_thickness)/2,-y/2+radius,wall_thickness+hook_thickness*2]) rotate([0,90,90]) linear_extrude(y-2*radius){
    polygon(points=[[0,0],[2*hook_thickness,0],[hook_thickness,hook_thickness]], center=true);
    }
}


module front_grove () {
    translate([(-x+2*radius)/2,-y/2+wall_thickness+tolerance/2,wall_thickness+hook_thickness*2]) rotate([90,90,90]) linear_extrude(x-2*radius){
    polygon(points=[[0,0],[2*hook_thickness,0],[hook_thickness,hook_thickness]], center=true);
    }
}

// Holes for pins
module pins_holes() {
    
    pins_w = 3;
    pins_off = 5;
    pins_r_x = 1;
    pins_r_h = 42;
    pins_l_h = 31;

    // Right pins
    x1 = (x/2) - wall_thickness - bar1_off - bar_w - pins_r_x - (pins_w / 2);
    // y1 = (y/2) - wall_thickness - pins_off - (pins_r_h / 2);
    y1 = (pins_r_h/2) - (y/2) + wall_thickness + pins_off;
    translate([x1,y1,-1]) linear_extrude(z) {
        square([pins_w, pins_r_h], center=true);
    }
    
    // Left pins
    x2 = x1 - 19;
    y2 = y1 - (pins_r_h - pins_l_h)/2;
    translate([x2,y2,-1]) linear_extrude(z) {
        square([pins_w, pins_l_h], center=true);
    }
}

module top_cover () {

    difference() {
        // Top face
        linear_extrude(wall_thickness){
            minkowski(){
                square([x-radius*2,y-radius*2], center=true);
                circle(radius, center=true);
            }
        }
        
        pins_holes();
    }
    
    difference(){
        // Wall of top cover
        linear_extrude(wall_thickness+hook_thickness*2){
            minkowski(){
                square([x-radius*2-wall_thickness*2-tolerance+wall_thickness*2,y-radius*2-wall_thickness*2-tolerance+wall_thickness*2], center=true);
                circle(radius-wall_thickness, center=true);
            }
        }
        
        // Hollow out
        // TODO: If radius is very small, still hollow out

        translate([0,0,wall_thickness]) linear_extrude(z){
            minkowski(){
                square([x-radius*2-wall_thickness*2-2*-tolerance+wall_thickness*2+top_cover_wall_thickness*2,y-radius*2-wall_thickness*2-2*top_cover_wall_thickness-tolerance+wall_thickness*2+top_cover_wall_thickness*2], center=true);
            circle(radius-wall_thickness-top_cover_wall_thickness);
            }
        }
        
        pins_holes();

    right_grove();
    rotate([180,180,0]) right_grove();
    front_grove();
    rotate([180,180,0])  front_grove();
    }
  

}

// left_hook();
print_part();

// For 2D print (Render, export to SVG and open with Inkscape)
// projection(cut = true) translate([0,0,0]) print_part();

module print_part() {
	if (part == "box") {
		bottom_box();
	} else if (part == "top") {
		top_cover();
	} else if (part == "both") {
		both();
	} else {
		both();
	}
}

module both() {
	translate([0,-(y/2+wall_thickness),0]) bottom_box();
    translate([0,+(y/2+wall_thickness),0]) top_cover();
}