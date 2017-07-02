#version 330

in		vec2	  v_TexCoord;			// varying variable for passing texture coordinate from vertex shader
uniform int       u_Wsize;				// blur width value
uniform int		  u_Hsize;				// blur height value
uniform float	  u_StepW;				// horizontal step
uniform float	  u_StepH;				// vertical step
uniform	sampler2D u_Sampler;			// uniform variable for the texture image

void main() {
	
	vec3 avg = vec3(0.0);				//average vector
	vec2 tc  = v_TexCoord;				//texture coordinate
	int  w2  = u_Wsize / 2;				//half width of the kernel
	int  h2  = u_Hsize / 2;				//half height of the kernel
	
	for(int i=-h2; i<=h2; ++i)
		for(int j = -w2; j <= w2; ++j)
			avg += texture2D(u_Sampler, vec2(tc.x + j*u_StepW, tc.y + i*u_StepH)).rgb;

	avg /= (u_Wsize*u_Hsize);
	gl_FragColor = vec4(avg, 1.0);		//put the average in the output image
}