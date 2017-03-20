uniform sampler2D colorTex;
uniform sampler2D ditherTex;
uniform vec2 step;

in vec2 uv;
out vec4 outcolor;

vec4 sample(vec2 p, float v) {
	return texture2D(colorTex, p+step*v);
}

void main() {
	float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;
	float sum = 16.f;
	vec4 accum = sample(uv, 0) * 6.f/sum;
	accum += sample(uv, 1) * 4.f/sum;
	accum += sample(uv,-1) * 4.f/sum;
	accum += sample(uv, 2) * 1.f/sum;
	accum += sample(uv,-2) * 1.f/sum;
	accum.a = clamp(accum.a, 0, 1);

	outcolor = accum;
	outcolor.rgb += vec3(dither);
}
