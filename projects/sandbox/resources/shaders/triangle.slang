// Task shader

struct MeshPayload {
    uint data;
};

[shader("amplification")]
[numthreads(1, 1, 1)]
void taskMain()
{
    MeshPayload p;
    p.data = 0;

    DispatchMesh(1, 1, 1, p);
}

// Mesh shader

const static float2 positions[3] = {
    float2( 0.0, -0.5),
    float2( 0.5,  0.5),
    float2(-0.5,  0.5)
};

const static float3 colors[3] = {
    float3(1.0, 1.0, 0.0),
    float3(0.0, 1.0, 1.0),
    float3(1.0, 0.0, 1.0)
};

struct Vertex
{
    float4 position : SV_Position;
    float3 color : Color;
};

const static uint NUM_VERTS = 3;
const static uint NUM_PRIMS = 1;

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(
    in MeshPayload meshPayload,
    OutputVertices<Vertex, NUM_VERTS> vertices,
    OutputIndices<uint3, NUM_PRIMS> indices)
{
    SetMeshOutputCounts(NUM_VERTS, NUM_PRIMS);

    vertices[0] = {float4(positions[0], 0.0, 1.0), colors[0]};
    vertices[1] = {float4(positions[1], 0.0, 1.0), colors[1]};
    vertices[2] = {float4(positions[2], 0.0, 1.0), colors[2]};

    indices[0] = uint3(0, 1, 2);
}

// Fragment Shader

struct Fragment
{
    float4 color : SV_Target;
};

[shader("fragment")]
Fragment fragMain(in Vertex vert)
{
	Fragment output;
	output.color = float4(vert.color, 1.0);
	return output;
}
