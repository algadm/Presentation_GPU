#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 speed;

/* ===== Uniforms ===== */
uniform float dt;
uniform mat4 Model;
uniform mat4 InvModel;
uniform vec3 origin;
uniform float mass;
uniform float radius;
uniform float reflect_coef;
uniform float friction_coef;

uniform sampler2D myDisplacementSampler;
uniform sampler2D myNormalSampler;

/* ===== Transform Feedback outputs ===== */
out vec3 pos;
out vec3 new_speed;

/* ===== Constants ===== */
const float PI = 3.14159265359;

/* ===== Gravity toward center ===== */
vec3 gravity_force(vec3 p)
{
    vec3 dir = normalize(origin - p);
    return dir * 9.8;
}

void main()
{
    /* ===== Transform to model space ===== */
    vec3 pos_model   = (InvModel * vec4(position, 1.0)).xyz;
    vec3 speed_model = (InvModel * vec4(speed,    0.0)).xyz;

    /* ===== Predict next position ===== */
    vec3 next_pos = pos_model + dt * speed_model;

    /* ===== Direction from center ===== */
    vec3 dir = normalize(next_pos - origin);

    /* ===== Spherical UV mapping ===== */
    float az = atan(dir.z, dir.x);          // [-pi, pi]
    float el = asin(clamp(dir.y, -1.0, 1.0)); // [-pi/2, pi/2]

    float u = 1.0 - (az / (2.0 * PI) + 0.5);
    float v = el / PI + 0.5;
    vec2 sphere_uv = vec2(u, v);

    /* ===== Displacement ===== */
    float displacement = texture(myDisplacementSampler, sphere_uv).r;
    float displaced_radius = radius + displacement * 0.2;

    /* ===== Normal mapping ===== */
    vec3 normal_ts = texture(myNormalSampler, sphere_uv).xyz * 2.0 - 1.0;

    vec3 T = normalize(vec3(-sin(az), 0.0, cos(az)));
    vec3 B = normalize(cross(dir, T));
    vec3 N = dir;

    mat3 TBN = mat3(T, B, N);
    vec3 normal_ws = normalize(TBN * normal_ts);

    /* ===== Collision test ===== */
    float dist_next = length(next_pos - origin);

    if (dist_next < displaced_radius)
    {
        /* ---- Collision response ---- */
        pos = origin + dir * (displaced_radius + 0.005);

        float vn = dot(speed_model, normal_ws);

        if (vn < 0.0)
        {
            vec3 v_n = vn * normal_ws;
            vec3 v_t = speed_model - v_n;
            new_speed = -reflect_coef * v_n + friction_coef * v_t;
        }
        else
        {
            new_speed = speed_model;
        }

        vec3 g = gravity_force(pos);
        g -= normal_ws * min(0.0, dot(g, normal_ws));
        new_speed += dt * mass * g;
    }
    else
    {
        /* ---- Free motion ---- */
        pos = next_pos;
        new_speed = speed_model + dt * mass * gravity_force(pos_model);
    }

    /* ===== Back to world space ===== */
    pos       = (Model * vec4(pos,       1.0)).xyz;
    new_speed = (Model * vec4(new_speed, 0.0)).xyz;
}
