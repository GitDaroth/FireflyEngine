#version 450

layout(triangles) in;

layout(line_strip, max_vertices=6) out;

layout(location = 0) in vec3 geomNormal[];
layout(location = 1) in mat4 mvp[];

void main()
{
    int i;
    for (i = 0; i < gl_in.length(); i++)
    {
        gl_Position = mvp[i] * gl_in[i].gl_Position;
        gl_Position.y = -gl_Position.y;
        EmitVertex();

        gl_Position = mvp[i] * (gl_in[i].gl_Position + vec4(geomNormal[i] * 0.1, 0.0));
        gl_Position.y = -gl_Position.y;
        EmitVertex();

        EndPrimitive();
    }
}