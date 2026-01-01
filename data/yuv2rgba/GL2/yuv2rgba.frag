//
// GL2
//

#version 120

uniform sampler2DRect uyvyTex;
uniform int colormatrix; // 0 = BT.601, 1 = BT.709, 2 - BT.2020

void main()
{
    // Full-width output pixel coordinates
    vec2 outCoord = gl_TexCoord[0].xy * vec2(2.0, 1.0);

    // Source UYVY (half width)
    vec2 srcCoord = vec2(floor(outCoord.x * 0.5), outCoord.y);
    vec4 uyvy = texture2DRect(uyvyTex, srcCoord);

    // Select Y0/Y1
    float Y = mod(floor(outCoord.x), 2.0) < 1.0 ? uyvy.g : uyvy.a;
	
	// Y limited in [16/255, 235/255] convert to full range
	Y = (Y - 16.0/255.0) * (255.0/(235.0-16.0));
	
	// Chroma
	float U = uyvy.r;
    float V = uyvy.b;

	// U and V limited in [16/255, 240/255]
	U = (U - 16.0/255.0) * (1.0 / ((240.0-16.0)/255.0));
	V = (V - 16.0/255.0) * (1.0 / ((240.0-16.0)/255.0));

    // Center chroma around 0
	U = U - 0.5;
    V = V - 0.5;
	
	// NDI docs
	// SD resolutions	BT.601
	// HD resolutions	Rec.709
	// UHD resolutions	Rec.2020

    vec3 rgb;
    if (colormatrix == 0) {
        // BT.601
        rgb.r = Y + 1.4020 * V;
        rgb.g = Y - 0.3441 * U - 0.7141 * V;
        rgb.b = Y + 1.7720 * U;
    } 
	else if (colormatrix == 1) {
        // BT.709
        rgb.r = Y + 1.5748 * V;
        rgb.g = Y - 0.1873 * U - 0.4681 * V;
        rgb.b = Y + 1.8556 * U;
    }
	else {
		// BT.2020
		rgb.r = Y + 1.4746 * V;
		rgb.g = Y - 0.16455 * U - 0.57135 * V;
		rgb.b = Y + 1.8814 * U;
	}
	
    gl_FragColor = vec4(clamp(rgb, 0.0, 1.0), 1.0);
}

