// Task shader

struct Payload {
    float2 origin;
    float2 offset_direction;
    float2 offset_direction_for_lods;
    float2 base_meshlet_extent;
};

const static uint NUM_MESHLETS_PER_PATCH_DIR = 2;
const static uint NUM_PATCHES_PER_DIR = 4; // Corresponds to LODs

[shader("amplification")]
[numthreads(1, 1, 1)]
void task_main(in uint3 gid : SV_GroupID)
{
    float2 direction;
    direction.x = gid.x == 0 ? 1.0 : -1.0;
    direction.y = gid.y == 0 ? 1.0 : -1.0;

    float2 offset_direction;
    offset_direction.x = min(direction.x, 0.0);
    offset_direction.y = min(direction.y, 0.0);

    float2 offset_direction_for_lods;
    switch (gid.z) {
    case 0:
        offset_direction_for_lods = float2(direction.x, 0.0);
        break;
    case 1:
        offset_direction_for_lods = direction;
        break;
    case 2:
        offset_direction_for_lods = float2(0.0, direction.y);
        break;
    }

    Payload payload;
    payload.origin = float2(0.0);
    payload.offset_direction = offset_direction;
    payload.offset_direction_for_lods = offset_direction_for_lods;
    payload.base_meshlet_extent = float2(1.0 / 16.0);

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
        float2 patch_offset = payload.origin;
        float2 patch_extent = meshlet_extent * NUM_MESHLETS_PER_PATCH_DIR;

        if (gid.z > 0) {
            meshlet_extent *= 1 << (gid.z - 1);
            patch_extent = meshlet_extent * NUM_MESHLETS_PER_PATCH_DIR;
            patch_offset += patch_extent * payload.offset_direction_for_lods;
        }

        patch_offset += patch_extent * payload.offset_direction;

        float2 meshlet_offset = patch_offset + meshlet_extent * gid.xy;
        float2 quad_extent = meshlet_extent / float(NUM_VERTS_PER_DIR - 1);

        float4 position = float4(meshlet_offset, 0.0, 1.0);
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

        indices[prim_idx] = tri;
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
