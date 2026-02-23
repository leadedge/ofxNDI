#version 120

//
//     YUV422_to_RGBA
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
// 2 pixels in 1 DWORD
//

uniform sampler2DRect tex;  // Input texture

float CLAMPRGB(float t) {
	float r = t;
	if(r > 1.) r = 1.;
	if(r < 0.) r = 0.;
	return r;
}

void main()
{
	vec2 currentPosition = gl_TexCoord[0].st;

	// Half width YUV422 source
	vec2 yuvPosition = vec2(currentPosition.x/2, currentPosition.y);

	// yuv pixel - 4 bytes u, y0, v, y1
	vec4 yuv = texture2DRect(tex, yuvPosition);	

	// u and v components are +-0.5
	float u  = yuv.x - .5;
	float v  = yuv.z - .5;

	// choose the right y - even if mod 2 < 1
	float y = yuv.y; // Even y0
	if(mod(yuvPosition.x, 2.0) > 1.)
		y  = yuv.w; // Odd y1

	// https://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
	// Given a reduced Y range 16-235 (220 steps) change to 0-255
	// y = (255/235)*(y-16)
	y = 1.16438*(y-0.06274);

	// Coefficients used by NDI Video Monitor
	// http://forums.newtek.com/showthread.php?155012-UYVY-gt-RGB-Conversion
	float r = CLAMPRGB(y - 0.0011*u + 1.7923*v);
	float g = CLAMPRGB(y - 0.2131*u - 0.5342*v);
	float b = CLAMPRGB(y + 2.1131*u - 0.0001*v);

	// No alpha
	gl_FragColor = vec4(r, g, b, 1.);

}
