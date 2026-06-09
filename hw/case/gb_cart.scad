// Gameboy Cartridge
//
// By: Dave Parker https://www.printables.com/social/175295-dave/about
//
// Print with build plate supports, 0.2mm layer height
//

include <BOSL/shapes.scad> // https://github.com/revarbat/BOSL

$fa = 1;
$fs = .1;
ff = 0.001; // fudge factor

width = 57;
depth = 64.8;

classic = false; // Whether to include a logo area on the top half

// fit test with top and bottom interlocking
// translate([0, 0, total_top_height + bottom_base_height + 1.8 ])rotate([0, 180, 0]) top();
// bottom();

// normal rendering
translate([-width/2 - 1, 0, 0]) top();
translate([width/2 + 1, 0, 0]) bottom();

translate([width/2 + 1, depth/2-0.7, 1.6]) % color("#ff000080") import("../pcb/SpiderCart.stl");
translate([-width/2 - 1, depth/2-0.7, 5.8]) rotate([0,180,0]) % color("#ff000080") import("../pcb/SpiderCart.stl");

// BOTTOM HALF /////////////////////////////////////////////////////////////////

alignment_post_height = 2;
alignment_post_r = 0.9;
alignment_post_y = 41.9;

bottom_base_height = 1.4;
bottom_wall_thickness = 1.8;
bottom_wall_height = 4.3;

far_notch_width = 2.5;
far_notch_depth = 1.2;
far_notch_height = 2.5;

groove_height = 1.4;
groove_width = 1.4;

lock_depth = 3;
lock_height = 3;
lock_travel = 2;
lock_width = 1;
lock_y_near = 11;
lock_y_far = 50;

pcb_screw_post_height = 1.6;
pcb_screw_post_r = 3.5;

ridge_depth = 0.8;
ridge_height = 0.2;
ridge_width = 50;

screw_head_height = 2;
screw_head_radius = 2.5;
screw_hole_passthrough_r = 1.1;
screw_y = 17;

type_notch_depth = 3;
type_notch_width = 5;

groove_depth = lock_y_near;
lock_entrance_height = bottom_wall_height - lock_height;
total_bottom_base_height = bottom_base_height + bottom_wall_height;


module bottom() {

    difference() { 
        union() {
            bottom_base();
            bottom_far_wall();
            bottom_right_wall();
            bottom_left_wall();
        }
        
        if(classic) {
            bottom_grip(classic_grip_y, classic_grip_count);
        } else {
            bottom_grip(grip_y, grip_count);
        }
        translate([-18, 67, 4]) cube([14, 10, 10], center=true);
        translate([-4, 67, 4]) cube([16, 10, 5.2], center=true);
    }
}


module bottom_base() {
    
    difference() { 
        union() {
            
            // Base minus notch at far end
            translate([-width/2, 0, 0])
                cuboid([width, depth - type_notch_depth + ff, bottom_base_height], fillet=0.5,
                    edges=EDGE_BK_RT+EDGE_BOT_LF+EDGE_BOT_RT, center=false);

            // Bit shorter at far end due to type notch
            translate([-width/2, depth - type_notch_depth - ff, 0])
                      cuboid([width - type_notch_width, type_notch_depth, bottom_base_height], fillet=0.5,
                      edges=EDGE_BK_RT+EDGE_BK_LF+EDGE_BOT_LF+EDGE_BOT_BK+EDGE_BOT_RT, center=false);
            
            // Ridge along inside bottom
            translate([-ridge_width/2, 0, bottom_base_height - ff])
                cube([ridge_width, ridge_depth, ridge_height]);
            
            // PCB screw post
            translate([0, screw_y, bottom_base_height - ff])
                cylinder(h=pcb_screw_post_height, r=pcb_screw_post_r);
            
            // Alignment post
            translate([0, alignment_post_y, bottom_base_height - ff])
                cylinder(h=alignment_post_height, r=alignment_post_r);
        }

        side_groove();  // Right
        mirror([-1, 0, 0]) side_groove(); // Left
        
        // Screw shaft hole
        translate([0, screw_y, - ff])
            cylinder(h=bottom_base_height + pcb_screw_post_height + ff*2, r=screw_hole_passthrough_r);

        // Screw thread hole
        translate([0, screw_y, - ff])
            cylinder(h=screw_head_height + ff*2, r=screw_head_radius);

    }
}


module bottom_grip(y, count) {
    
    for(i=[1:1:count]) {
        
        dy = y + i*(grip_thickness + grip_spacing);

         translate([-width/2 - ff, dy, - ff])
             cube([grip_side_depth, grip_thickness, total_bottom_base_height  + ff*2]);
        
         translate([width/2 - grip_side_depth + ff , dy, - ff])
             cube([grip_side_depth, grip_thickness, total_bottom_base_height + ff*2]);
    }
}


module side_groove() {
    translate([ridge_width/2, - ff, groove_height/2 + ff]) 
        cube([groove_width, groove_depth, groove_height/2 + ff*2]);
}


module bottom_right_wall() {  

    difference() {
        union() {
            translate([width/2 - bottom_wall_thickness, 0, bottom_base_height - ff])
                cuboid([bottom_wall_thickness, depth - type_notch_depth, bottom_wall_height],
                        fillet=0.5, edges=EDGE_BK_RT, center=false);

            bottom_wall_common_additive();
        }

        bottom_wall_common_subtractive();
    }
}


module bottom_left_wall() {  

    difference() {
        union() {
            translate([-width/2, 0, bottom_base_height - ff])
                cube([bottom_wall_thickness, depth - bottom_wall_thickness + ff, bottom_wall_height]);
            
            mirror([-1, 0, 0]) bottom_wall_common_additive();
        }
        
        mirror([-1, 0, 0]) bottom_wall_common_subtractive();
    }
}


module bottom_wall_common_additive() {
    
    dx = width/2 - bottom_wall_thickness - (ridge_width/2 + groove_width) + ff;
    translate([ridge_width/2 + groove_width, 0, bottom_base_height - ff])
        cube([dx, groove_depth, bottom_wall_height]);
}


module bottom_wall_common_subtractive() {
    lock(lock_y_near);
    lock(lock_y_far);
}


module lock(y) {
    
    // Entrance
    translate([width/2 - bottom_wall_thickness - ff, y, bottom_base_height - ff])
        cube([lock_width, lock_depth, bottom_wall_height + ff*2]);
    
    // Travel
    translate([width/2 - bottom_wall_thickness - ff, y + lock_depth - ff, bottom_base_height - ff])
        cube([lock_width, lock_travel, lock_height + ff*2]); 
}


module bottom_far_wall() {
    
    difference() {
        // Far wall
        translate([-width/2, depth - bottom_wall_thickness, bottom_base_height - ff])
            cuboid([width - type_notch_width, bottom_wall_thickness, bottom_wall_height],
            fillet=0.5, edges=EDGE_BK_LF+EDGE_BK_RT, center=false);
        
        // Far wall notch
        translate([-far_notch_width/2, depth - bottom_wall_thickness - ff, bottom_base_height - ff])
            cube([far_notch_width, far_notch_depth, far_notch_height]);
    }
 
    // Mode notch horizontal wall
    translate([width/2 - bottom_wall_thickness - type_notch_width + ff,
              depth - bottom_wall_thickness - type_notch_depth,
              bottom_base_height - ff])
        cube([type_notch_width, bottom_wall_thickness,  bottom_wall_height]);
    
    // Mode notch vertical wall
    translate([width/2 - bottom_wall_thickness - type_notch_width + ff,
              depth - bottom_wall_thickness - type_notch_depth,
              bottom_base_height - ff])
        cube([bottom_wall_thickness, type_notch_depth,  bottom_wall_height]);
}


// TOP HALF /////////////////////////////////////////////////////////////////////

far_tab_width = far_notch_width - 0.5;
far_tab_depth = far_notch_depth - 0.2;
far_tab_height = far_notch_height/2; 

classic_grip_count = 5;

grip_count = 6;
grip_side_depth = 0.3;
grip_top_depth = 0.6;
grip_thickness = 1;
grip_spacing = 1;

key_depth = lock_depth - 1;
key_width = lock_width - 0.2;

label_depth = 39;
label_height = 0.6;
label_y = 8;
label_width = 44;

logo_depth = 12;
logo_height = 0.6;
logo_width = 44;
logo_y = label_y + label_depth + 2;

screw_height = 3;
screw_hole_thread_r = 0.9;
screw_shaft_r = 2;

side_tab_y = 32;
side_tab_width = 1.5;
side_tab_depth = 1;

top_angle_cut_depth = 37.7;
top_height = 2;
top_inner_height = 1.3;
top_near_wall_y = 10;
top_wall_height = 3.8;
top_wall_thickness = 1.5;

wall_gap = 0.4; // Padding between top and bottom walls

grip_y = label_y + label_depth;
classic_grip_y = grip_y + 1.5;
key_height = top_wall_height - lock_entrance_height - 0.75;
total_top_height = top_height - top_inner_height + top_wall_height;


module top() {
    
    difference() {
        union() {
            top_base();
            
            if(classic) {
                // Logo border
                translate([-logo_width/2 - 1, logo_y - 1, 0])
                    cuboid([logo_width + 2, logo_depth + 2, logo_height],
                           fillet=7, edges=EDGES_Z_ALL, center=false);
                
            }
        }
        
        if(classic) {
            // Logo cutout
            translate([-logo_width/2, logo_y, -ff])
                cuboid([logo_width, logo_depth, logo_height],
                       fillet=6, edges=EDGES_Z_ALL, center=false);
        }

        translate([18, 66, 4]) cube([14, 10, 10], center=true);
        translate([4, 66, 4]) cube([16, 10, 5.2], center=true);
    }
}


module top_base() {

    difference() {
        union() {
            
            // Bit with beveled sides
            translate([0, top_angle_cut_depth/2, 0])
                prismoid(size1=[width - top_height*2, top_angle_cut_depth],
                         size2=[width, top_angle_cut_depth],
                         h=top_height,
                         center=false);
            
            // Middle rectangular bit
            translate([-width/2, top_angle_cut_depth - ff, 0])
                cuboid([width, depth - top_angle_cut_depth - type_notch_depth + 2*ff, top_height], fillet=0.5,
                    edges=EDGE_BK_LF+EDGE_BOT_LF+EDGE_BOT_RT, center=false);

            // Bit shorter end due to notch.
            translate([-width/2 + type_notch_width, depth - type_notch_depth - ff, 0])
                cuboid([width - type_notch_width, type_notch_depth, top_height], fillet=0.5,
                    edges=EDGE_BK_RT+EDGE_BK_LF+EDGE_BOT_BK+EDGE_BOT_RT+EDGE_BOT_LF, center=false);
        }
        
        if(classic) {
            top_grip(classic_grip_y, classic_grip_count);
        } else {
            top_grip(grip_y, grip_count);
        }
         
        // Thinner inset
        translate([-(width - top_height*2 - wall_gap*2)/2, - ff, top_inner_height + ff])
            cube([width - top_height*2 - wall_gap*2,
                 depth - bottom_wall_thickness - wall_gap,
                 top_height - top_inner_height]);

        // Label cutout
        translate([-label_width/2, label_y, - ff])
            cuboid([label_width, label_depth, label_height], fillet=1, edges=EDGES_Z_ALL, center=false);
        
        // Insert arrow cutout
        translate([0, 2, - ff]) linear_extrude(label_height) polygon([[-4.5, 5], [0, 0],[ 4.5, 5]]);
            
        // Screw hole (removed)
        //translate([0, screw_y, - ff*2])
        //    cylinder(h=top_inner_height + ff*4, r=screw_hole_thread_r);   

    }

    // Near wall
    top_near_wall_width = width - bottom_wall_thickness*2 - wall_gap*2;
    translate([-top_near_wall_width/2, top_near_wall_y, top_inner_height - ff])
        cube([top_near_wall_width, top_wall_thickness, top_wall_height]);
    
    top_side_wall();
    
    difference() {
        union() {
            
            // Far wall
            translate([-(width - bottom_wall_thickness*2 - wall_gap*2)/2,
                       depth - bottom_wall_thickness - top_wall_thickness - wall_gap,
                       top_inner_height - ff])
                cube([width - bottom_wall_thickness*2 - wall_gap*2, top_wall_thickness, top_wall_height]);
            
            // Far wall tab
            translate([-far_tab_width/2,
                      depth - bottom_wall_thickness - wall_gap - ff,
                      top_inner_height + top_wall_height - far_tab_height])
                cube([far_tab_width, far_tab_depth, far_tab_height]);
            
            mirror([-1, 0, 0]) top_side_wall(add_tab=false); // Remove tab due to memory chip on V1 board
        }
        
        // Mode notch cutout
        translate([-(width - bottom_wall_thickness*2 - wall_gap*2)/2 - ff,
                   depth - bottom_wall_thickness - top_wall_thickness - wall_gap - type_notch_depth,
                   top_inner_height - ff*2])
            cube([type_notch_width + top_wall_thickness,
                  type_notch_depth + top_wall_thickness + ff*2,
                  top_wall_height + ff*2]);
    }
    
    // Fill hole in base where walls removed
    difference() {
       translate([-(width - bottom_wall_thickness*2 - wall_gap*2)/2,
                  depth - bottom_wall_thickness - wall_gap - type_notch_depth, top_inner_height])
            cube([type_notch_width, type_notch_depth, top_height - top_inner_height]);
        
        translate([-width/2 + ff, depth - type_notch_depth - ff, - ff ])
            cube([type_notch_width, type_notch_depth, top_wall_height]);
    }
    
    // Type notch inset walls
    notch_wall_y = depth - bottom_wall_thickness - top_wall_thickness - wall_gap - type_notch_depth - ff;
    translate([-(width - bottom_wall_thickness*2 - wall_gap*2)/2,
           notch_wall_y,
           top_inner_height - ff])
        cube([top_wall_thickness + type_notch_width, top_wall_thickness, top_wall_height]);
    
    translate([-(width - bottom_wall_thickness*2 - wall_gap*2)/2 + type_notch_width,
           notch_wall_y,
           top_inner_height - ff])
        cube([top_wall_thickness, top_wall_thickness + type_notch_depth, top_wall_height]);

    difference() {
        // Screw shaft
        translate([0, screw_y, top_inner_height - ff])
            cylinder(h=screw_height, r=screw_shaft_r);
        
        // Screw hole
        translate([0, screw_y, top_inner_height - ff*2])
            cylinder(h=screw_height + ff*4, r=screw_hole_thread_r);    
    }
}


module top_grip(y, count) {
    
    for(i=[1:1:count]) {
        
        dy = y + i*(grip_thickness + grip_spacing);
         translate([-width/2, dy, - ff])
             cube([width, grip_thickness, grip_top_depth]);

         translate([-width/2 - ff, dy, - ff])
             cube([grip_side_depth, grip_thickness, top_height + ff*2]);
        
         translate([width/2 - grip_side_depth + ff , dy, - ff])
             cube([grip_side_depth, grip_thickness, top_height + ff*2]);
    }
}


module top_side_wall(add_tab=true) {

    translate([width/2 - bottom_wall_thickness - wall_gap - top_wall_thickness,
               top_near_wall_y - ff,
               top_inner_height - ff])
        cube([top_wall_thickness,
              depth - bottom_wall_thickness - top_wall_thickness - wall_gap - top_near_wall_y + ff*2,
              top_wall_height]);
    
    // Side tab
    if (add_tab)
    translate([width/2 - bottom_wall_thickness - wall_gap - top_wall_thickness - side_tab_width + ff,
              side_tab_y,
              top_inner_height - ff])
        cube([side_tab_width, side_tab_depth, top_wall_height]);
    
    key(lock_y_near);
    key(lock_y_far);
}


key_y = function (lock_y) lock_y + lock_travel + (lock_depth - key_depth)/2;


module key(lock_y) {
    
    translate([width/2 - bottom_wall_thickness - top_wall_thickness - wall_gap - ff,
               key_y(lock_y),
               top_inner_height + top_wall_height - key_height
              ])
    cube([key_width + top_wall_thickness, key_depth, key_height]);
}
