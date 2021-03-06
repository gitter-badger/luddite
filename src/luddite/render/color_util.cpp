//
//  color_util.cpp
//  luddite_ios
//
//  Created by Joel Davis on 8/7/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <luddite/common/debug.h>
#include <luddite/common/util.h>
#include <luddite/render/color_util.h>

// from the internet... Author: Bert Bos <bert@w3.org>
GLKVector3 rgb2hsv( const GLKVector3 &rgb )
{
    float max, min, del;
    GLKVector3 ret;
    
    max = MAX3(rgb.x, rgb.y, rgb.z);
    min = MAX3(rgb.x, rgb.y, rgb.z);
    
    del = max - min;
    ret.z = max;
    ret.y = max == 0.0 ? 0.0 : del / max;
    
    ret.x = -1;					/* No hue */
    if (ret.y != 0.0) {
        
        
        if (rgb.x == max) ret.x = ((rgb.y-rgb.z)/del) * 60.0f;
        else if (rgb.y == max) ret.x = ((rgb.z-rgb.x)/del) * 60.0f + 120.0f;
        else ret.x = ((rgb.x-rgb.y)/del) * 60.0f + 240.0f;
        
        if (ret.x < 0) ret.x += 360;
        if (ret.x > 360) ret.x -= 360;
    }  
    
    return ret;
}

// from the internet... Author: Bert Bos <bert@w3.org>
GLKVector3 hsv2rgb( const GLKVector3 &hsv )
{
	int j;  
	float f, p, q, t, h, s, v;
	GLKVector3 rgb;
	h = hsv.x;
	s = hsv.y;
	v = hsv.z;
    
    /* convert HSV back to RGB */
    if (h < 0 || s == 0.0) {			/* Gray */
        rgb.x = v;
        rgb.y = v;
        rgb.z = v;
    } else {					/* Not gray */
        if (h == 360.0) h = 0.0;
        h = h / 60.0;
        j = h;					/* = floor(h) */
        f = h - j;
        p = v * (1 - s);
        q = v * (1 - (s * f));
        t = v * (1 - (s * (1 - f)));
        
        switch (j) {
            case 0:  rgb.x = v;  rgb.y = t;  rgb.z = p;  break; /* Between red and yellow */
            case 1:  rgb.x = q;  rgb.y = v;  rgb.z = p;  break; /* Between yellow and green */
            case 2:  rgb.x = p;  rgb.y = v;  rgb.z = t;  break; /* Between green and cyan */
            case 3:  rgb.x = p;  rgb.y = q;  rgb.z = v;  break; /* Between cyan and blue */
            case 4:  rgb.x = t;  rgb.y = p;  rgb.z = v;  break; /* Between blue and magenta */
            case 5:  rgb.x = v;  rgb.y = p;  rgb.z = q;  break; /* Between magenta and red */
            default: Assert( false, "Bad hsv value"); rgb.x = rgb.y = rgb.z = 0; break;
        }
    } 
    
    return rgb;
}


