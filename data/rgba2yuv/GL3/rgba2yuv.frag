//
// GL3
//

#version 150

//
//     RGBA to YUV422
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
//

uniform sampler2DRect rgbatex; // rgba source texture
uniform int colormatrix; // 0 = BT.601, 1 = BT.709, 2 = BT.2020

in vec2 texCoord;
out vec4 outputColor;

void main()
{
	// RGBA texture is twice as wide as the source YUV texture
	vec4 rgba0 = texture(rgbatex, vec2(texCoord.x*2, texCoord.y));
	vec4 rgba1 = texture(rgbatex, vec2(texCoord.x*2+1, texCoord.y));
	
	// Average two pixels for U/V (4:2:2)
	vec3 avg = (rgba0.rgb + rgba1.rgb)*0.5;
	
	// NDI docs:
    // SD  > BT.601
    // HD  > BT.709
    // UHD > BT.2020
	
	float y0, y1, u, v;
	if (colormatrix == 0) {
        // BT.601
		y0 =  0.299000*rgba0.r + 0.587000*rgba0.g + 0.114000*rgba0.b;
		y1 =  0.299000*rgba1.r + 0.587000*rgba1.g + 0.114000*rgba1.b;
		u  = -0.167836*avg.r - 0.331264*avg.g + 0.500000*avg.b;
		v  =  0.500000*avg.r - 0.418688*avg.g - 0.081312*avg.b;		
	}
	else if (colormatrix == 1) {
		// BT.709
		y0 =  0.2126*rgba0.r + 0.7152*rgba0.g + 0.0722*rgba0.b;
		y1 =  0.2126*rgba1.r + 0.7152*rgba1.g + 0.0722*rgba1.b;
		u  = -0.1146*avg.r - 0.3854*avg.g + 0.5000*avg.b;
		v  =  0.5000*avg.r - 0.4542*avg.g - 0.0458*avg.b;	
	}
	else {
		// BT.2020
		y0 =  0.2627*rgba0.r + 0.6780*rgba0.g + 0.0593*rgba0.b;
		y1 =  0.2627*rgba1.r + 0.6780*rgba1.g + 0.0593*rgba1.b;
		u  = -0.1396*avg.r - 0.3604*avg.g + 0.5000*avg.b;
		v  =  0.5000*avg.r - 0.4600*avg.g - 0.0400*avg.b;		
	}

	// Convert Y from 0-255 (full range) to 16-235 (studio range)
	// 0 - 1  to  0.06274 - 0.92156
	
	// Y
	y0 = y0 * 0.858823 + 0.062745;
	y1 = y1 * 0.858823 + 0.062745;
	
	// U, V
	u = u * 0.878431;
	v = v * 0.878431;

	// Adjust u and v to 0-1 range
	u += 0.5;
	v += 0.5;

	// Pack as UYVY
    outputColor = vec4(u, y0, v, y1);

}
