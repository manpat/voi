uniform sampler2D depthTex;
uniform sampler2D colorTex;
uniform sampler2D general0Tex;
uniform sampler2D general1Tex;
uniform sampler2D ditherTex;
uniform mat4 invProjection;
uniform mat4 prevProjView;
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
	float depth0 = texture2D(depthTex, uv).r;
	float dither = texture2D(ditherTex, uv).r / 255.0 * 4.0;

	// mat4 invVP = inverse(projection*view);
	// vec4 cpw = inverse(projection) * vec4(uv*2 - 1, depth0*2-1, 1);
	// cpw /= cpw.w;
	// vec4 wpw = inverse(view) * cpw;
	// vec4 oldwp = inverse(prevProjView) * cpw;

	// vec3 wsvel = vec3(sin(time+wpw.y*0.1),cos(time*3+wpw.y+wpw.z),0) * cpw.z*0.02; //wpw.xyz - oldwp.xyz;
	// if(length(wsvel) > 1.f) wsvel = normalize(wsvel);

	// vec3 csvel = (view * vec4(wsvel*0.1, 0)).xyz;
	// vec4 uvvel = projection * vec4(csvel, 0);
	// uvvel /= uvvel.w;

	// outcolor.rgb = wsvel*.5 + .5;
	// outcolor.a = 1.f;
	// return;
	// vec3 wp = wpw.xyz;

	// float dd = 2.f;
	// vec2 texSize = dd/textureSize(depthTex, 0);
	vec2 uv0 = uv; // + uvvel.xy*0.1f;
	// vec2 uv1 = uv0+vec2(texSize.x,0);
	// vec2 uv2 = uv0+vec2(0,texSize.y);
	// depth0 = texture2D(depthTex, uv0).r;
	// float depth1 = texture2D(depthTex, uv1).r;
	// float depth2 = texture2D(depthTex, uv2).r;

	vec4 cpw0 = invProjection * vec4(uv0*2 - 1, depth0*2-1, 1);
	vec3 cp0 = cpw0.xyz/cpw0.w;

	vec4 color = texture2D(colorTex, uv0);
	vec4 particle = texture2D(general0Tex, uv0);

	float fogdepth = length(cp0) + dither; // Distance from eye
	// float fogmix = 1-clamp(pow(fogdepth / fogDistance, fogColor.a), 0, 1) * color.a;
	float fogmix = 1-min(pow(fogdepth / fogDistance, fogColor.a), 1) * color.a;

	vec3 bloom = texture2D(general1Tex, uv).rgb;

	// outcolor.rgb = clamp(color.rgb*fogmix + fogColor.rgb*(1-fogmix), 0, 1);
	outcolor.rgb = color.rgb*fogmix + fogColor.rgb*(1-fogmix);
	outcolor.rgb += particle.rgb * particle.a;

	outcolor.rgb += bloom * 0.03;
	outcolor.rgb = applyTonemap(outcolor.rgb, 7.0 + 0.0*pow(2.0 + sin(time * 0.1), 2.0));

	float vignette = clamp(1 - pow(length(uv-.5), vignettePower) * vignetteStrength, 0.3, 1);
	outcolor.rgb *= vignette + dither;
	outcolor.a = color.a;
}
