#version 400 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 lightSpaceMatrices[6];

out vec4 FragPos;

void main()
{
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