#version 400 core

layout (triangles) in;
layout (line_strip, max_vertices = 3) out;

layout(location = 0) in VS_OUT {
    mat4 geomProj;
} gs_in[];

out vec3 fragNormal;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return -normalize(cross(a, b));
}

vec4 GetCentroid()
{
    return (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;
}

void main()
{
	fragNormal = gs_in[0].geomProj * GetNormal();

	// Iterate over cubemap faces
    for (int faceId = 0; faceId < 6; ++faceId)
	{
		gl_Layer = faceId;
		
		for (int vertexId = 0; vertexId < 3; ++vertexId)
		{
			FragPos = gl_in[vertexId].gl_Position;
			gl_Position = lightSpaceMatrices[faceId] * FragPos;
			EmitVertex();
		}
		
		EndPrimitive();
	}
}