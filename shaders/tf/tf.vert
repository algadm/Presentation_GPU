#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 speed;

uniform vec3 origin;
uniform float mass;
uniform float radius;
uniform float reflect_coef;
uniform float friction_coef;
uniform sampler2D myDisplacementSampler;
uniform sampler2D myNormalSampler;

out vec3 pos;
out vec3 new_speed;

const float dt = 0.001;

// generate pseudo-random vector from speed
vec3 rand(vec3 s) {
    return vec3(
        fract(sin(dot(s.xyz ,vec3(12.9898,78.233,144.7272))) * 43758.5453),
        fract(sin(dot(s.zxy ,vec3(12.9898,78.233,144.7272))) * 43758.5453),
        fract(sin(dot(s.yxz ,vec3(12.9898,78.233,144.7272))) * 43758.5453)
    );
}

// gravity that points toward the center
vec3 gravity_force(vec3 pos) {
    vec3 dir = normalize(origin - pos);
    float strength = 9.8;   // adjust if you want
    return dir * strength;
}

void main()
{
    // Compute gravity toward center
    vec3 g_force = gravity_force(position);

    // Compute next position with current velocity
    vec3 next_pos = position + dt * speed;

    // Compute spherical UV coordinates based on next_pos (or position)
    vec3 dir = normalize(next_pos - origin);
    float PI = 3.14159265359;

    // Compute azimuth angle az in [-pi, pi]
    float az = atan(dir.z, dir.x);

    // Map az to [0,1] and flip u to match mesh
    float u_tex = 1.0 - (az / (2.0 * PI) + 0.5);

    // Compute elevation angle el in [-pi/2, pi/2] and map to [0,1]
    float el = asin(dir.y);
    float v_tex = (el / PI) + 0.5;

    vec2 sphere_uv = vec2(u_tex, v_tex);



    // Sample displacement texture using spherical UVs
    float displacement = texture(myDisplacementSampler, sphere_uv).r;
    float displaced_radius = radius + displacement * 0.2f;

    vec3 normal_ts = texture(myNormalSampler, sphere_uv).xyz;
    normal_ts = normal_ts * 2.0 - 1.0;

    vec3 T = normalize(vec3(-sin(az), 0.0, cos(az)));
    vec3 B = normalize(cross(dir, T));
    vec3 N = dir;    // radial normal
    mat3 TBN = transpose(mat3(T, B, N));
    vec3 normal_ws = normalize(TBN * normal_ts);


    float dist_next = length(next_pos - origin);

    if (dist_next < displaced_radius)
    {
        // Clamp position to displaced sphere surface
        pos = origin + dir * (displaced_radius + 0.05f);

        vec3 n_s = dot(speed, normal_ws) * normal_ws;
        vec3 t_s = speed - n_s;
        new_speed  = -reflect_coef * n_s + friction_coef * t_s;
    }
    else
    {
        // No collision, move freely
        pos = next_pos;
        new_speed = speed + dt * mass * g_force;
    }
}