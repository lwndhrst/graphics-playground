struct PushConstants {
    float4x4 mvp;

    // Dynamic mesh generation and culling
    float2 origin;         // Projection of camera position on xz-plane
    float2 view_direction; // Projection of viewing direction on xz-plane
    float cutoff_angle;

    // Displacement calculation
    float t;
};

[vk::push_constant] uniform PushConstants pc;

// Task shader

struct Payload {
    float2 base_meshlet_extent;
    float2 offset_direction;
    float2 offset_direction_for_lods;
};

const static uint NUM_MESHLETS_PER_PATCH_DIR = 16;
const static uint NUM_PATCHES_PER_DIR = 16; // Each patch corresponds to one LOD

const static uint NUM_TASK_THREADS = 64;

[shader("amplification")]
[numthreads(NUM_TASK_THREADS, 1, 1)]
void task_main(
    in uint gid : SV_GroupID,
    in uint gtid : SV_GroupThreadID)
{
    // Only dispatch mesh shaders once per workgroup
    if (gtid == 0) {

        // 4 quadrants with 3 directions each for a total of 12 workgroups
        //
        //   -------
        //  |\  |  /|
        //  | \ | / |
        //  |  \|/  |
        //  |-------|
        //  |  /|\  |
        //  | / | \ |
        //  |/  |  \|
        //   -------

        float2 direction;
        switch (gid % 4) {
        case 0:
            direction = float2(1.0);
            break;
        case 1:
            direction = float2(1.0, -1.0);
            break;
        case 2:
            direction = float2(-1.0, 1.0);
            break;
        case 3:
            direction = float2(-1.0);
            break;
        }

        float2 offset_direction;
        offset_direction.x = min(direction.x, 0.0);
        offset_direction.y = min(direction.y, 0.0);

        float2 offset_direction_for_lods;
        switch (gid % 3) {
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

        // Cull based on viewing direction and fov
        float2 v = -normalize(pc.view_direction);
        float2 d = normalize(direction + offset_direction_for_lods);
        bool cull = dot(d, v) < cos(pc.cutoff_angle);

        if (!cull) {
            Payload payload;
            payload.base_meshlet_extent = float2(1.0 / 1024.0);
            payload.offset_direction = offset_direction;
            payload.offset_direction_for_lods = offset_direction_for_lods;

            DispatchMesh(NUM_MESHLETS_PER_PATCH_DIR, NUM_MESHLETS_PER_PATCH_DIR, NUM_PATCHES_PER_DIR, payload);
        }
    }
}

// Mesh shader

struct Vertex {
    float4 position : SV_Position;
    float3 color;
};

const static uint NUM_VERTS_PER_DIR = 9;
const static uint NUM_VERTS = NUM_VERTS_PER_DIR * NUM_VERTS_PER_DIR;
const static uint NUM_PRIMS = (NUM_VERTS_PER_DIR - 1) * (NUM_VERTS_PER_DIR - 1) * 2;

const static uint NUM_MESH_THREADS = NUM_VERTS > NUM_PRIMS ? NUM_VERTS : NUM_PRIMS; // 128 with this configuration

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

    if (gtid < NUM_VERTS) {
        float2 meshlet_extent = payload.base_meshlet_extent;
        float2 patch_offset = pc.origin;
        float2 patch_extent = meshlet_extent * NUM_MESHLETS_PER_PATCH_DIR;

        if (gid.z > 0) {
            meshlet_extent *= 1 << (gid.z - 1);
            patch_extent = meshlet_extent * NUM_MESHLETS_PER_PATCH_DIR;
            patch_offset += patch_extent * payload.offset_direction_for_lods;
        }

        patch_offset += patch_extent * payload.offset_direction;

        float2 meshlet_offset = patch_offset + meshlet_extent * gid.xy;
        float2 quad_extent = meshlet_extent / float(NUM_VERTS_PER_DIR - 1);

        // Position in xz-plane rather than xy
        float4 position = float4(meshlet_offset.x, 0.0, meshlet_offset.y, 1.0);
        position.x += float(gtid % NUM_VERTS_PER_DIR) * quad_extent.x;
        position.z += float(gtid / NUM_VERTS_PER_DIR) * quad_extent.y;

        // Calc height displacement using sine wave for now
        position.y = 0.1 * sin(pc.t + position.x + 0.5 * position.z);

        vertices[gtid].position = mul(pc.mvp, position);
        vertices[gtid].color = float3((gid.xy + 1) / float(NUM_MESHLETS_PER_PATCH_DIR), 0.5);
    }

    if (gtid < NUM_PRIMS) {
        uint3 tri = gtid % 2 == 0
            ? uint3(0, 1, NUM_VERTS_PER_DIR)
            : uint3(NUM_VERTS_PER_DIR + 1, NUM_VERTS_PER_DIR, 1);

        // tri.x = 0                 + (gtid & 1) * (NUM_VERTS_PER_DIR + 1);
        // tri.y = 1                 + (gtid & 1) * (NUM_VERTS_PER_DIR - 1);
        // tri.z = NUM_VERTS_PER_DIR - (gtid & 1) * (NUM_VERTS_PER_DIR - 1);

        tri += uint3(gtid >> 1);
        tri += uint3((gtid >> 1) / (NUM_VERTS_PER_DIR - 1));

        indices[gtid] = tri;
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
