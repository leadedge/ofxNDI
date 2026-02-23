//
// GL2 - RGBA > UYVY
//

#version 120

//
//     RGBA to YUV422 (UYVY)
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
//

uniform sampler2DRect rgbatex; // rgba source texture
uniform int colormatrix; // 0 = BT.601, 1 = BT.709, 2 = BT.2020

void main()
{
	vec2 currentPosition = gl_TexCoord[0].xy;

	// RGBA texture is twice as wide as the source YUV texture
	vec4 rgba0 = texture2DRect(rgbatex, vec2(currentPosition.x*2, currentPosition.y));
	vec4 rgba1 = texture2DRect(rgbatex, vec2(currentPosition.x*2+1, currentPosition.y));
	
	// Average two pixels for U/V (4:2:2)
	vec3 avg = (rgba0.rgb + rgba11.rgb)*0.5;
	
	// NDI docs
    // SD  > BT.601
    // HD  > BT.709
    // UHD > BT.2020
	
	float y0, y1, u, v;
	if (colormatrix == 0) {
		// BT.601
		y0 =  0.299000*rgba0.r + 0.587000*rgba0.g + 0.114000*rgba0.b;
		y1 =  0.299000*rgba1.r + 0.587000*rgba1.g + 0.114000*rgba1.b;
		// u  = -0.167836*rgba0.r - 0.331264*rgba0.g + 0.500000*rgba0.b;
		// v  =  0.500000*rgba0.r - 0.418688*rgba0.g - 0.081312*rgba0.b;		
		// These work better with only a slight color shift
		// 10 100 140 -> 10 100 141
		u = -0.168935*avg.r - 0.331665*avg.g + 0.500600*avg.b; // ??? + 0.5;
		v =  0.499813*avg.r - 0.418531*avg.g - 0.081282*avg.b; // + 0.5;
	}
	else if (colormatrix == 1) {
		// BT.709
		y0 =  0.2126*rgba0.r + 0.7152*rgba0.g + 0.0722*rgba0.b;
		y1 =  0.2126*rgba1.r + 0.7152*rgba1.g + 0.0722*rgba1.b;
		// u  = -0.1146*rgba0.r - 0.3854*rgba0.g + 0.5000*rgba0.b;
		// v  =  0.5000*rgba0.r - 0.4542*rgba0.g - 0.0458*rgba0.b;	
		u  = -0.1146*avg.r - 0.3854*avg.g + 0.5000*avg.b;
		v  =  0.5000*avg.r - 0.4542*avg.g - 0.0458*avg.b;	
		// These work better
		// 10 100 140 -> 10 100 141
		// u  = -0.1146*avg.r - 0.3854*avg.g + 0.5000*avg.b; // + 0.5;
		// v =   0.5000*avg.r - 0.4542*avg.g - 0.0458*avg.b; // + 0.5;
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
	// y = (y/1.16438)+0.06274
	y0 = y0/1.16438 + 0.06274;
	y1 = y1/1.16438 + 0.06274;

	// Adjust u and v to 0-1 range
	u += 0.5;
	v += 0.5;

	// u y0 v y1
	vec4 yuv422 = vec4(0.0);
	yuv422.x = u;
	yuv422.y = y0;
	yuv422.z = v;
	yuv422.w = y1;

	gl_FragColor = yuv422;

}
