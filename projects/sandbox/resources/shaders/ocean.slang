// Task shader

struct Payload {
    float2 origin;
    float2 global_direction;
    float2 local_direction;
    float quad_extent;
};

[shader("amplification")]
[numthreads(1, 1, 1)]
void task_main(in uint3 gid : SV_GroupID)
{
    float2 global_direction;
    global_direction.x = gid.x == 0 ? 1.0 : -1.0;
    global_direction.y = gid.y == 0 ? 1.0 : -1.0;

    float2 local_direction;
    switch (gid.z) {
    case 0:
        local_direction = float2(global_direction.x, 0.0);
        break;
    case 1:
        local_direction = global_direction;
        break;
    case 2:
        local_direction = float2(0.0, global_direction.y);
        break;
    }

    Payload payload;
    payload.origin = float2(0.0, 0.0);
    payload.global_direction = global_direction;
    payload.local_direction = local_direction;
    payload.quad_extent = 0.125;

    DispatchMesh(4, 1, 1, payload);
}

// Mesh shader

struct Vertex {
    float4 position : SV_Position;
    float3 color : Color;
};

const static uint NUM_VERTS_X = 2;
const static uint NUM_VERTS_Y = 2;
const static uint NUM_VERTS = NUM_VERTS_X * NUM_VERTS_Y;
const static uint NUM_PRIMS = 2;

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void mesh_main(
    in uint3 gid : SV_GroupID,
    in uint gtid : SV_GroupThreadID,
    in payload Payload payload,
    OutputVertices<Vertex, NUM_VERTS> vertices,
    OutputIndices<uint3, NUM_PRIMS> indices)
{
    SetMeshOutputCounts(NUM_VERTS, NUM_PRIMS);

    float2 offset = payload.origin;
    float2 extent = payload.quad_extent;

    if (gid.x == 1) {
        offset += extent * payload.local_direction;
    }

    if (gid.x > 1) {
        extent *= 2 << (gid.x - 2);
        offset += extent * payload.local_direction;
    }

    extent *= payload.global_direction;

    float2 v0 = offset;
    float2 v1 = offset + float2(extent.x, 0);
    float2 v2 = offset + float2(0, extent.y);
    float2 v3 = offset + extent;

    vertices[0].position = float4(v0, 0.0, 1.0);
    vertices[1].position = float4(v1, 0.0, 1.0);
    vertices[2].position = float4(v2, 0.0, 1.0);
    vertices[3].position = float4(v3, 0.0, 1.0);

    vertices[0].color = float3(abs(v0), 0.0);
    vertices[1].color = float3(abs(v1), 0.0);
    vertices[2].color = float3(abs(v2), 0.0);
    vertices[3].color = float3(abs(v3), 0.0);

    if (sign(payload.global_direction.x) == sign(payload.global_direction.y)) {
        indices[0] = uint3(0, 1, 2);
        indices[1] = uint3(1, 3, 2);
    }
    else {
        indices[0] = uint3(0, 2, 1);
        indices[1] = uint3(1, 2, 3);
    }
}

// Fragment Shader

struct Fragment {
    float4 color : SV_Target;
};

[shader("fragment")]
Fragment frag_main(in Vertex vert)
{
    Fragment output;
    output.color = float4(vert.color, 1.0);
    return output;
}
