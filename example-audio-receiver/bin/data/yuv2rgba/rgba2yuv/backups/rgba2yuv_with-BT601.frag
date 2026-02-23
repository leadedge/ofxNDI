#version 120

//
//     RGBA to YUV422
//
// Y sampled at every pixel
// U and V sampled at every second pixel 
// 2 pixels in 1 DWORD


uniform sampler2DRect tex;  // yuv texture to draw to
uniform sampler2DRect rgbatex;  // rgba texture twice as wide
uniform vec2 texturesize;

float CLAMP(float t) {
	float r = t;
	if(r > 1.) r = 1.;
	if(r < 0.) r = 0.;
	return r;
}

void main()
{
	vec2 currentPosition = gl_TexCoord[0].st;

	vec4 rgba0 = texture2DRect(rgbatex, vec2(currentPosition.x*2, currentPosition.y));
	vec4 rgba1 = texture2DRect(rgbatex, vec2(currentPosition.x*2+1, currentPosition.y));

	float y0 = 0;
	float y1 = 0;
	float u  = 0;
	float v  = 0;

	/*
	if(texturesize.y <= 576) { // SD 720×576 PAL
		// http://www.martinreddy.net/gfx/faqs/colorconv.faq
		// BT609
		// International standard for digital coding
		// of TV pictures at 525 and 625 line rates
		//
		// Y'= 0.299*R' + 0.587*G' + 0.114*B'
		// Cb=-0.169*R' - 0.331*G' + 0.500*B'
		// Cr= 0.500*R' - 0.419*G' - 0.081*B'
		y0 =  0.299*rgba0.r + 0.587*rgba0.g + 0.114*rgba0.b;
		y1 =  0.299*rgba1.r + 0.587*rgba1.g + 0.114*rgba1.b;
		u  = -0.169*rgba0.r - 0.331*rgba0.g + 0.500*rgba0.b;
		v  =  0.500*rgba0.r - 0.419*rgba0.g - 0.081*rgba0.b;
	}
	else { // HD : 720p, 1080i, 1080p, 1440p
	*/
		// BT.709
		// ITU.BT-709 HDTV studio production in Y'CbCr
		//
		// Y'= 0.2215*R' + 0.7154*G' + 0.0721*B'
		// Cb=-0.1145*R' - 0.3855*G' + 0.5000*B'
		// Cr= 0.5016*R' - 0.4556*G' - 0.0459*B'
		y0 =  0.2215*rgba0.r + 0.7154*rgba0.g + 0.0721*rgba0.b;
		y1 =  0.2215*rgba1.r + 0.7154*rgba1.g + 0.0721*rgba1.b;
		u  = -0.1145*rgba0.r - 0.3855*rgba0.g + 0.5000*rgba0.b;
		v  =  0.5016*rgba0.r - 0.4556*rgba0.g - 0.0459*rgba0.b;
	// }

	// Coefficients used by NDI Video Monitor
	// http://forums.newtek.com/showthread.php?155012-UYVY-gt-RGB-Conversion
	// r = y - 0.0011*u + 1.7923*v
	// g = y - 0.2131*u - 0.5342*v
	// b = y + 2.1131*u - 0.0001*v



	// Convert Y from 0-255 to 16-235
	// 0 - 1  to  0.06274 - 0.92156
	// y = 1.16438*(y0-0.06274) - for yuv to rgba
	// y = (y/1.16438)+0.06274 - for rgba to yuv
	y0 = y0/1.16438 + 0.06274;
	y1 = y1/1.16438 + 0.06274;

	// Adjust u and v to work on GPU
	u += 0.5;
	v += 0.5;

	// u y0 v y1
	vec4 yuva = vec4(0.0);
	yuva.x = u;
	yuva.y = y0;
	yuva.z = v;
	yuva.w = y1;

	gl_FragColor = yuva;


}
