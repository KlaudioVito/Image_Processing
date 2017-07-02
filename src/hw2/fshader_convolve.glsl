#version 330

in		vec2	  v_TexCoord;			// varying variable for passing texture coordinate from vertex shader

uniform int       u_SizeW;				//uniform int for kernel width
uniform int		  u_SizeH;				//uniform int for kernel height
uniform float	  u_StepX;				//input image horizontal step
uniform float	  u_StepY;				//input image vertical step
uniform float 	  u_Weights[100];		//uniform array of float kernel values
uniform	sampler2D u_Sampler;			//uniform variable for the texture image

void main() {
	vec3 conv = vec3(0.0);				//convolution vector
	vec2 tc  = v_TexCoord;				//texture coordinate
	int  w2  = u_SizeW / 2;				//half width of the kernel
	int  h2  = u_SizeH / 2;				//half height of the kernel
	int k = 0;							//current kernel value

	//perform convolution: multiply texture value by kernel value
	for(int i=-h2; i<=h2; ++i)
		for(int j = -w2; j <= w2; ++j)
			conv += (texture2D(u_Sampler, vec2(tc.x + j*u_StepX, tc.y + i*u_StepY)).rgb) * u_Weights[k++];

	gl_FragColor = vec4(conv, 1.0);		//put the convolution in the output image
}
