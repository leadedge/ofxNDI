#version 150

//
//     RGBA to YUV422
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
//

uniform sampler2DRect tex; // fbo texture to draw to
uniform sampler2DRect rgbatex; // rgba source texture

in vec2 texCoord;
out vec4 outputColor;

void main()
{
	// vec2 currentPosition = texcoord[0].st;

	// RGBA texture is twice as wide as the source YUV texture
	vec4 rgba0 = texture2DRect(rgbatex, vec2(texCoord.x*2, texCoord.y));
	vec4 rgba1 = texture2DRect(rgbatex, vec2(texCoord.x*2+1, texCoord.y));

	// BT.709
	// ITU.BT-709 HDTV studio production in Y'CbCr
	//
	// http://www.martinreddy.net/gfx/faqs/colorconv.faq
	//
	// Y'= 0.2215*R' + 0.7154*G' + 0.0721*B'
	// Cb=-0.1145*R' - 0.3855*G' + 0.5000*B'
	// Cr= 0.5016*R' - 0.4556*G' - 0.0459*B'

	float y0 =  0.2215*rgba0.r + 0.7154*rgba0.g + 0.0721*rgba0.b;
	float y1 =  0.2215*rgba1.r + 0.7154*rgba1.g + 0.0721*rgba1.b;
	float u  = -0.1145*rgba0.r - 0.3855*rgba0.g + 0.5000*rgba0.b;
	float v  =  0.5016*rgba0.r - 0.4556*rgba0.g - 0.0459*rgba0.b;

	// Convert Y from 0-255 to 16-235
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

	outputColor = yuv422;

}
