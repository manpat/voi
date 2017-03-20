uniform sampler2D colorTex;
uniform sampler2D general0Tex;
uniform sampler2D general1Tex;
uniform sampler2D ditherTex;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 fogColor;
uniform float fogDistance;
uniform float vignettePower;
uniform float vignetteStrength;
uniform float time;
in vec2 uv;

out vec4 outcolor;

vec3 Uncharted2ToneMapping(vec3 color, float exposure) {
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;

	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;

	return color;
}

vec3 filmicToneMapping(vec3 color) {
	color = max(vec3(0.0), color - vec3(0.004));
	color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color) {
	float white = 2.0;
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma * (1.0 + luma / (white*white)) / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return color;
}

vec3 applyTonemap(vec3 color, float exposure) {
	const float gamma = 2.2;

	// Exposure tone mapping
	// vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
	// Reinhard tone mapping
	// vec3 mapped = color / (color + vec3(1.0));

	// vec3 mapped = color * exposure / (color/exposure + vec3(1.0));

	// float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	// vec3 mapped = exposure * color / (luma / exposure + vec3(1.0));

	vec3 mapped = Uncharted2ToneMapping(color, exposure);
	// vec3 mapped = filmicToneMapping(color);
	// vec3 mapped = whitePreservingLumaBasedReinhardToneMapping(color);

	// Gamma correction 
	// mapped = pow(mapped, vec3(1.0 / gamma));

	return mapped;
}

void main() {
	float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;

	vec4 color = texture2D(colorTex, uv);
	vec4 particle = texture2D(general0Tex, uv);

	vec3 bloom = texture2D(general1Tex, uv).rgb;

	outcolor.rgb = color.rgb;
	outcolor.rgb += particle.rgb * particle.a;

	outcolor.rgb += bloom;
	outcolor.rgb = applyTonemap(outcolor.rgb, 7.0 + 0.0*pow(2.0 + sin(time * 0.1), 2.0));

	float vignette = clamp(1 - pow(length(uv-.5), vignettePower) * vignetteStrength, 0.3, 1);
	outcolor.rgb *= vignette + dither;
	outcolor.a = color.a;
}
