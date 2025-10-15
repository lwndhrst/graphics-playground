// Task shader

struct Payload {
    float2 global_direction;
    float2 local_direction;
    float2 origin;
    float2 base_meshlet_extent;
};

const static uint NUM_MESHLETS_PER_PATCH_DIR = 4;
const static uint NUM_PATCHES_PER_DIR = 4; // corresponds to LODs

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
    payload.global_direction = global_direction;
    payload.local_direction = local_direction;
    payload.origin = float2(0.0);
    payload.base_meshlet_extent = float2(1.0 / 32.0);

    DispatchMesh(NUM_MESHLETS_PER_PATCH_DIR, NUM_MESHLETS_PER_PATCH_DIR, NUM_PATCHES_PER_DIR, payload);
}

// Mesh shader

struct Vertex {
    float4 position : SV_Position;
    float3 color;
};

const static uint NUM_VERTS_PER_DIR = 9;
const static uint NUM_VERTS = NUM_VERTS_PER_DIR * NUM_VERTS_PER_DIR;
const static uint NUM_PRIMS = (NUM_VERTS_PER_DIR - 1) * (NUM_VERTS_PER_DIR - 1) * 2;
const static uint NUM_MESH_THREADS = NUM_VERTS > NUM_PRIMS ? NUM_VERTS : NUM_PRIMS;

[shader("mesh")]
[numthreads(NUM_MESH_THREADS, 1, 1)]
[outputtopology("triangle")]
void mesh_main(
    in uint3 gid : SV_GroupID,
    in uint gtid : SV_GroupThreadID,
    in payload Payload payload,
    OutputVertices<Vertex, NUM_VERTS> vertices,
    OutputIndices<uint3, NUM_PRIMS> indices)
{
    SetMeshOutputCounts(NUM_VERTS, NUM_PRIMS);

    uint vert_idx = gtid;
    uint prim_idx = gtid;

    if (vert_idx < NUM_VERTS) {
        float2 meshlet_extent = payload.base_meshlet_extent;
        float2 meshlet_offset = payload.origin;

        if (gid.z > 0) {
            meshlet_extent *= 1 << (gid.z - 1);
            meshlet_offset += meshlet_extent * payload.local_direction * NUM_MESHLETS_PER_PATCH_DIR;
        }

        meshlet_extent *= payload.global_direction;

        float2 quad_extent = meshlet_extent / float2(NUM_VERTS_PER_DIR - 1);
        float2 quad_offset = meshlet_offset + meshlet_extent * float2(gid.xy);

        float4 position = float4(quad_offset, 0.0, 1.0);
        position.x += float(gtid % NUM_VERTS_PER_DIR) * quad_extent.x;
        position.y += float(gtid / NUM_VERTS_PER_DIR) * quad_extent.y;

        vertices[vert_idx].position = position;
        vertices[vert_idx].color = float3((gid.xy + 1) / float(NUM_MESHLETS_PER_PATCH_DIR), 0.5);
    }

    if (prim_idx < NUM_PRIMS) {
        uint3 tri;

        if(prim_idx % 2 == 0) {
            tri = uint3(0, 1, NUM_VERTS_PER_DIR);
        }
        else {
            tri = uint3(NUM_VERTS_PER_DIR + 1, NUM_VERTS_PER_DIR, 1);
        }

        tri += uint3(prim_idx >> 1);
        tri += uint3((prim_idx >> 1) / (NUM_VERTS_PER_DIR - 1));

        if (sign(payload.global_direction.x) == sign(payload.global_direction.y)) {
            indices[prim_idx] = tri.xyz;
        }
        else {
            indices[prim_idx] = tri.xzy;
        }
    }
}

// Fragment Shader

struct Fragment {
    float4 color : SV_Target;
};

[shader("fragment")]
Fragment frag_main(in Vertex vert)
{
    Fragment frag;
    frag.color = float4(vert.color, 1.0);

    return frag;
}
