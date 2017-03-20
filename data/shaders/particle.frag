uniform vec3 materialColor;
in float vlifetime;

out vec4 outcolor;

void main() {
	if(vlifetime <= 0.f) discard;

	// vec2 diff = vec2(.5f) - gl_PointCoord;
	// float dist = .6f-(abs(diff.x)+abs(diff.y))/*.5f - length()*/;
	// dist = pow(clamp(dist, 0.f, 1.f), 0.9f);
	float dist = 0.35f;
	float a = sin(radians(vlifetime*180.f)) * dist;
	outcolor = vec4(materialColor*a, a);
}