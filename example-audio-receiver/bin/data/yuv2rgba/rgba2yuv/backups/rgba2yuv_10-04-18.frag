#version 120

//
//     RGBA to YUV422
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
// 2 pixels in 1 DWORD
//
//	R = Y + 1.403V
//	G = Y - 0.344U - 0.714V
//	B = Y + 1.770U
//

uniform sampler2DRect tex;  // rgba texture
uniform sampler2DRect yuvtex;  // yuv texture
uniform vec2 textureSize;

float CLAMP(float t) {
	float r = t;
	if(r > 1.) r = 1.;
	if(r < 0.) r = 0.;
	return r;
}

void main()
{
	vec2 currentPosition = gl_TexCoord[0].st;

	/*
	// Half width YUV422
	vec2 yuvPosition = vec2(currentPosition.x/2, currentPosition.y);

	// rgba pixel - 4 bytes r, g, b, a
	vec4 col = texture2DRect(tex, currentPosition);	

	float r = col.r;
	float g = col.g;
	float b = col.b;

	// HDTV with BT.709
	// https://en.wikipedia.org/wiki/YUV#HDTV_with_BT.709
	
	// Y = 0.2126*r + 0.7152*g + 0.0722*b;
	// U = -0.09991*r -0.33609*g + 0.436*b;
	// V = 0.615*r -0.55861*g -0.05638*b;

	// R = Y + 1.28033*U;
	// G = Y - 0.21482*U - 0.38059*V;
	// B = Y + 2.12798*U;
	 
	float y = CLAMP(0.2126*r + 0.7152*g + 0.0722*b);
	float u = -0.09991*r -0.33609*g + 0.436*b;
	float v = 0.615*r -0.55861*g -0.05638*b;

	// yuv pixel - 4 bytes u, y0, v, y1
	// u and v are +-0.5 - convert to 0-1
	u += .5;
	v += .5;
	u = CLAMP(u);
	v = CLAMP(v);

	// yuv pixel - 4 bytes u, y0, v, y1
	// half as many as rgba

	// No alpha
	gl_FragColor = vec4(u, y, v, y);
	*/

	/*
	// https://github.com/libretro/glsl-shaders/blob/master/nnedi3/shaders/rgb-to-yuv.glsl
	vec4 rgba = texture2DRect(tex, currentPosition);	
	vec4 yuva = vec4(0.0);

	yuva.x = rgba.r * 0.299 + rgba.g * 0.587 + rgba.b * 0.114;
	yuva.y = rgba.r * -0.169 + rgba.g * -0.331 + rgba.b * 0.5 + 0.5;
	yuva.z = rgba.r * 0.5 + rgba.g * -0.419 + rgba.b * -0.081 + 0.5;
	yuva.w = 1.0;
	
	gl_FragColor = yuva;
	*/

	// https://www.xaymar.com/2017/07/06/how-to-converting-rgb-to-yuv-and-yuv-to-rgb/
	// BT.709
	// It can further be adapted to use a matrix instead of fixed values
	// vec4 rgba = texture2DRect(tex, currentPosition);
	
	vec4 yuva = texture2DRect(yuvtex, currentPosition);
	
	// yuva.r = rgba.r * 0.2126 + 0.7152 * rgba.g + 0.0722 * rgba.b;
	// yuva.g = (rgba.b - yuva.r) / 1.8556;
	// yuva.b = (rgba.r - yuva.r) / 1.5748;
	// yuva.a = rgba.a;
	// Adjust to work on GPU
	// yuva.gb += 0.5;
	// gl_Position = vec2(currentPosition.x/2, currentPosition.y);

	gl_FragColor = yuva;

	// gl_FragColor = vec4(1., 0., 0., 1.);


}
