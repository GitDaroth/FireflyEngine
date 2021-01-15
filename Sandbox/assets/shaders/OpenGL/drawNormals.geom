#version 450

layout(triangles) in;

layout(line_strip, max_vertices=6) out;

in vec3 geomNormal[];
in mat4 mvp[];

void main()
{
	int i;
	for (i = 0; i < gl_in.length(); i++)
	{
		gl_Position = mvp[i] * gl_in[i].gl_Position;
		EmitVertex();

		gl_Position = mvp[i] * (gl_in[i].gl_Position + vec4(geomNormal[i] * 5.0, 0.0));
		EmitVertex();

		EndPrimitive();
	}
}