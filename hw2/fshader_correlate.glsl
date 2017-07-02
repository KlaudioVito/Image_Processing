#version 330

in vec2           v_TexCoord;   // varying variable for passing texture coordinate from vertex shader
uniform float     u_StepX;
uniform float     u_StepY;
uniform sampler2D u_Sampler;    // uniform variable for the texture image
uniform int       u_SizeW_T;
uniform int       u_SizeH_T;
uniform float     u_StepX_T;
uniform float     u_StepY_T;
uniform sampler2D u_Sampler_T;
uniform float     u_Sqrt_Sum_T;


void main() {
    vec4 corr = vec4(0.0);
    vec4 sum = vec4(0.0);
    vec2 tc  = v_TexCoord;
    int  sizeW  = u_SizeW_T / 2;
    int  sizeH  = u_SizeH_T / 2;
    int count = 0;

    for(int i=-sizeH; i<=sizeH; ++i) {
        for(int j=-sizeW; j<=sizeW; ++j) {
            float val = texture2D(u_Sampler, vec2(tc.x + j*u_StepX, tc.y + i*u_StepY));
			float val_T = texture2D(u_Sampler_T, vec2(0.5 + j*u_StepX_T, 0.5 + i*u_StepY_T));	
			corr +=  val * val_T;
            sum += val*val;
		}
    }
    gl_FragColor = vec4(corr.rgb/sqrt(sum.rgb)/u_Sqrt_Sum_T, 1.0);
}
