#version 120

//
//     RGBA to YUV422 (UYVY)
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
//

uniform sampler2DRect tex;     // fbo texture to draw to
uniform sampler2DRect rgbatex; // rgba source texture

void main()
{
	// fbo texture pixel screen coords
	vec2 currentPosition = gl_TexCoord[0].xy;
	
	// Get the pixel color from the rgba texture
	// U and V from the first of every second pixel
	// Y0 and Y1 luminance from each of the pair
	vec4 rgba0 = texture2DRect(rgbatex, vec2(currentPosition.x*2, currentPosition.y));
	vec4 rgba1 = texture2DRect(rgbatex, vec2(currentPosition.x*2+1, currentPosition.y));
	
	// Calculate Y0 Y1 U V
	// NDI uses Rec.719 for 720p and 1920p
	//
	// BT.709
	// https://gist.github.com/yohhoy/dafa5a47dade85d8b40625261af3776a
	// https://www.itu.int/rec/R-REC-BT.709
	// Y  = 0.2126*R + 0.7152*G + 0.0722*B
	// Cb = (B-Y) / 1.8556
	// Cr = (R-Y) / 1.5748
	//
	float y0 =  0.2126*rgba0.r + 0.7152*rgba0.g + 0.0722*rgba0.b;
	float y1 =  0.2226*rgba1.r + 0.7152*rgba1.g + 0.0722*rgba1.b;
	float u  =  (rgba0.b-y0) / 1.8556;
	float v  =  (rgba0.r-y0) / 1.5748;
	
	// Convert Y from 0-255 to 16-235
	// (0-1 to 0.06274-0.92156)
	//  y = (y/1.16438)+0.06274
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
