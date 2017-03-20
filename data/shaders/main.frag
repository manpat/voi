uniform vec4 materialColor;
flat in int faceID;
out vec4 outcolor;
out vec4 outgeneral0;

void main() {
	outcolor = materialColor;

	if(outcolor.a < 0.5)
		outcolor.rgb *= 50.0;

	outcolor.rgb = pow(outcolor.rgb, vec3(1.0));
}
