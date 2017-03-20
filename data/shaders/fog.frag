uniform sampler2D depthTex;
uniform sampler2D colorTex;
uniform sampler2D ditherTex;
uniform mat4 invProjection;
uniform vec4 fogColor;
uniform float fogDistance;
in vec2 uv;

out vec4 outcolor;

void main() {
	vec4 color = texture2D(colorTex, uv);
	float depth = texture2D(depthTex, uv).r;
	float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;

	vec4 cpw0 = invProjection * vec4(uv*2 - 1, depth*2-1, 1);
	vec3 cp0 = cpw0.xyz/cpw0.w;

	float fogdepth = length(cp0) + dither; // Distance from eye
	float fogmix = 1-clamp(pow(fogdepth / fogDistance, fogColor.a), 0, 1) * color.a;

	vec3 fogcolor = fogColor.rgb;

	// fogcolor.rgb += 0.3;

	outcolor.rgb = color.rgb*fogmix + fogcolor.rgb*(1-fogmix);
	outcolor.a = color.a;
}