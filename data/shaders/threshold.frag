uniform sampler2D colorTex;
in vec2 uv;

out vec4 outcolor;

void main() {
	vec3 c = texture2D(colorTex, uv).rgb;
	float luminance = dot(c.rgb, vec3(0.2125, 0.7154, 0.0721));

	outcolor = vec4(0,0,0,1);
	if(luminance > 1.0)
		outcolor.rgb = c;
}