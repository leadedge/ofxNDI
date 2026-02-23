#version 120

//
//     YUV422_to_RGBA
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
// 2 pixels in 1 DWORD
//
//	R = Y + 1.403V
//	G = Y - 0.344U - 0.714V
//	B = Y + 1.770U
//

uniform sampler2DRect tex;  // Input texture
uniform vec2 textureSize;

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

	// u and v components
	float u  = yuv.x;
	float v  = yuv.z;

	// u and v are +-0.5
	u -= .5;
	v -= .5;

	// choose the right y
	float y = yuv.y;
	// even if mod 2 < 1
	if(mod(yuvPosition.x, 2.0) < 1.)
		y  = yuv.w;

	// https://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
	// Given a reduced Y range 16-235 (220 steps)
	// 16/255  = 0.0627450980392157
	// 255/235 = 1.164383561643836
	// y = (255/235)*(y-16)
	// for y=16 : y = 0
	// for y = 235 : y = (255/235)*(235-16) = 1.164*219 = 255
	// y = 1.16438*(y-0.06274);

	// BT.709 
	// https://www.xaymar.com/2017/07/06/how-to-converting-rgb-to-yuv-and-yuv-to-rgb/
	// http://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf
	// float r = CLAMPRGB(y + 1.5748*v);
	// float g = CLAMPRGB(y - 0.187324*u - 0.468124*v);
	// float b = CLAMPRGB(y + 1.8556*u);

	// Original C code
	// https://www.fourcc.org/fccyvrgb.php
	// The JFIF file format that is commonly used with JPEG 
	// defines a YCbCr color space that is slightly different than usual,
	// in that all components have the full [0,255] excursion rather than
	// [16,235] and [16,240]. These equations are for this variant and
	// are very close to the ones in JFIF:
	// float r = CLAMPRGB(y + 1.403*v);
	// float g = CLAMPRGB(y - 0.344*u - 0.714*v);
	// float b = CLAMPRGB(y + 1.770*u);



	// y = 1.1643*(y-0.0625);
	// float r = CLAMPRGB(y + 1.5958*v);
	// float g = CLAMPRGB(y - 0.39173*u - 0.81290*v);
	// float b = CLAMPRGB(y + 2.017*u);

	// R = 1.164(Y - 16) + 1.596(V - 128)
	// G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
	// B = 1.164(Y - 16) + 2.018(U - 128)

	// Or for 0-1 scale instead of 0-255
	// y = 1.164*(y - 0.0627);
	// float r = CLAMPRGB(y + 1.596*v);
	// float g = CLAMPRGB(y - 0.391*u - 0.813*v);
	// float b = CLAMPRGB(y + 2.018*u);


	// SD resolutions	BT.601
	// HD resolutions	Rec.709
	// UHD resolutions	Rec.2020

	// SD
	// BT601
	// ITU-T Rec.601 defines different color primaries for 625-line systems
	// (as used in most PAL systems) and for 525-line systems 
	// (as used in the SMPTE 170M-2004 standard for NTSC)

	// http://www.martinreddy.net/gfx/faqs/colorconv.faq
	//  R'= Y' + 0.000*U' + 1.403*V'
    //	G'= Y' - 0.344*U' - 0.714*V'
    //	B'= Y' + 1.773*U' + 0.000*V'
	// float r = CLAMPRGB(y + 1.403*v);
	// float g = CLAMPRGB(y - 0.344*u - 0.714*v);
	// float b = CLAMPRGB(y + 1.773*u);

	// HD
	// Rec.709 (BT709)
	// http://www.martinreddy.net/gfx/faqs/colorconv.faq
	//	R'= Y' + 0.0000*Cb + 1.5701*Cr
    //	G'= Y' - 0.1870*Cb - 0.4664*Cr
    //	B'= Y' - 1.8556*Cb + 0.0000*Cr
	// float r = CLAMPRGB(y + 1.5701*v);
	// float g = CLAMPRGB(y - 0.1870*u - 0.4664*v);
	// float b = CLAMPRGB(y + 1.8556*u);

	// UHD (BT.2020)
	/*
	https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/ColorSpaceUtility.hlsli

float3 REC709toREC2020( float3 RGB709 )
{
    static const float3x3 ConvMat =
    {
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    };
    return mul(ConvMat, RGB709);
}
*/

// https://patches.videolan.org/patch/14145/
// RGB-709 to RGB-2020 based on https://www.researchgate.net/publication/258434326_Beyond_BT709

// http://avisynth.nl/index.php/Colorimetry

// https://www.khronos.org/registry/DataFormat/specs/1.2/dataformat.1.2.html#PRIMARIES_BT2020

R2020 G2020 B2020)≈(1.716651, −0.355671, −0.253366
                   −0.666684,  1.616481,  0.015769
				    0.017640, −0.042771,  0.942103

R601SMPTE
G601SMPTE
B601SMPTE)≈( 3.506003, −1.739791, −0.544058
            −1.069048,  1.977779,  0.035171
			 0.056307, −0.196976,  1.049952

	// float r = CLAMPRGB(y + 1.5748*v);
	// float g = CLAMPRGB(y - 0.187324*u - 0.468124*v);
	// float b = CLAMPRGB(y + 1.8556*u);


	// NDI
	float r = CLAMPRGB(y - 0.0011*u + 1.7923*v);
	float g = CLAMPRGB(y - 0.2131*u - 0.5342*v);
	float b = CLAMPRGB(y + 2.1131*u - 0.0001*v);

	// NDI original
	// http://forums.newtek.com/showthread.php?155012-UYVY-gt-RGB-Conversion
	// mat3 ycbcr_to_rgb_mat = mat3(
		// 1.16414, -0.0011, 1.7923,
		// 1.16390, -0.2131, -0.5342,
		// 1.16660, 2.1131, -0.0001);
	// vec3 ycbcr_to_rgb_vec = vec3(-0.9726, 0.3018, -1.1342);

	// Y, U, V
	// vec3 col_rgb = vec3( dot( ycbcr_to_rgb_mat[0], col_y_cbcr ), dot( ycbcr_to_rgb_mat[1], col_y_cbcr ), dot( ycbcr_to_rgb_mat[2], col_y_cbcr ) ) + ycbcr_to_rgb_vec;
	// gl_FragColor = vec4( col_rgb, 1.0 );
	// float r = CLAMPRGB(1.16414*(y-0.06274) - 0.0011*u + 1.7923*v);
	// float g = CLAMPRGB(1.16390*(y-0.06274) - 0.2131*u - 0.5342*v);
	// float b = CLAMPRGB(1.16660*(y-0.06274) + 2.1131*u - 0.0001*v);


	// No alpha
	gl_FragColor = vec4(r, g, b, 1.);

}
