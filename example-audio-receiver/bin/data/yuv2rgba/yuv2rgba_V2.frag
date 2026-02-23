#version 120

//
//     YUV422_to_RGBA
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
// 2 pixels in 1 DWORD
//

// Resolution loss horizontally compared to NDI Video Monitor
// TODO : YUVA for alpha - see NDI SDK documentation

uniform sampler2DRect tex; // fbo RGBA texture to write to
uniform sampler2DRect yuvtex; // YUV source texture

/*
float CLAMPRGB(float t) {
	float r = t;
	if(r > 1.) r = 1.;
	if(r < 0.) r = 0.;
	return r;
}
*/

void main()
{

	// OK 
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	return;
	
	
	/*
	vec2 currentPosition = gl_TexCoord[0].st;
    vec4 colour = texture2DRect(tex, currentPosition);	
	vec4 colourInverted = vec4(colour.b, colour.g, colour.r, colour.a);
	gl_FragColor = colourInverted;
	*/
	
	
	vec2 currentPosition = gl_TexCoord[0].xy;
	
	// vec4 c = texture2DRect(yuvtex, currentPosition);
	// gl_FragColor = c; // vec4(c.r, c.g, c.b, 1.0);


	// Half width YUV422 source
	vec2 yuvPosition = vec2(currentPosition.x/2.0, currentPosition.y);

	// yuv pixel - 4 bytes u, y0, v, y1
	vec4 yuv = texture2DRect(yuvtex, yuvPosition);	

	// u and v components are +-0.5
	float u = yuv.r - .5;
	float v = yuv.b - .5;
	float y = yuv.a; // Odd column y1

	// choose the correct y - even column if mod 2 < 1
	if(mod(currentPosition.x, 2.0) < 1.)
		y = yuv.g; // Even column y0

	// NDI docs
	// SD resolutions	BT.601
	// HD resolutions	Rec.709
	// UHD resolutions	Rec.2020

	// BT.709 
	// https://www.xaymar.com/2017/07/06/how-to-converting-rgb-to-yuv-and-yuv-to-rgb/
	// http://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf
	// https://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
	// Given a reduced Y range 16-235 (220 steps) change to 0-255
	// y = (255/235)*(y-16)
	// y = 1.16438*(y-0.06274);
	// float r = CLAMPRGB(y + 1.5748*v);
	// float g = CLAMPRGB(y - 0.187324*u - 0.468124*v);
	// float b = CLAMPRGB(y + 1.8556*u);

	// Coefficients and luminance adjustment used by NDI Video Monitor
	// http://forums.newtek.com/showthread.php?155012-UYVY-gt-RGB-Conversion
	float r = CLAMPRGB(1.16414*(y-0.06274) - 0.0011*u + 1.7923*v);
	float g = CLAMPRGB(1.16390*(y-0.06274) - 0.2131*u - 0.5342*v);
	float b = CLAMPRGB(1.16660*(y-0.06274) + 2.1131*u - 0.0001*v);
	
	// vec4 c = vec4(r, g, b, 1.0);
	// gl_FragColor = c;

	// No alpha
	// gl_FragColor = vec4(r, g, b, 1.0);
	// gl_FragColor = vec4(y, y, y, 1.); // resolution test
	gl_FragColor = vec4(1., 0., 0., 1.);
	
}
