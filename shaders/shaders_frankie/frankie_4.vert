#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 v_pos;

uniform mat4 MVP;
uniform float frankie_cam_theta;
uniform float frankie_cam_phi;

/* =========================
   Rotations utilitaires
   ========================= */

mat3 rotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat3(
        vec3( c, 0.0, -s),
        vec3(0.0, 1.0,  0.0),
        vec3( s, 0.0,  c)
    );
}

mat3 rotationX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat3(
        vec3(1.0, 0.0,  0.0),
        vec3(0.0,  c,    s),
        vec3(0.0, -s,    c)
    );
}

/* =========================
   MAIN
   ========================= */

void main()
{
    float radius = 3.0;
    vec3 center = vec3(0.0);
    vec3 worldUp;

    // 1. Rotation locale pour dresser le mesh et inverser Y
    mat3 localRotation = rotationX(3.14159265) * rotationY(2.52);

    // 2. Scale uniforme 0.5 pour réduire de 2
    vec3 localPos = localRotation * position * 0.5;

    // 3. Position orbitale
    vec3 orbitPos;
    orbitPos.x = radius * cos(frankie_cam_phi) * cos(frankie_cam_theta);
    orbitPos.y = radius * sin(frankie_cam_phi);
    orbitPos.z = radius * cos(frankie_cam_phi) * sin(frankie_cam_theta);

    // 4. Calcul de la direction vers le centre
    vec3 forward = normalize(center - orbitPos);

    // 5. Choix de l’up global
    if (abs(forward.y) > 0.99)
    {
        worldUp = vec3(0.0, 0.0, 1.0);
    }
    else
    {
        worldUp = vec3(0.0, 1.0, 0.0);
    }

    // 6. Base orthonormée
    vec3 right = normalize(cross(worldUp, forward));
    vec3 up    = cross(forward, right);

    mat3 lookAtRotation;
    lookAtRotation[0] = right;
    lookAtRotation[1] = up;
    lookAtRotation[2] = forward;

    // 7. Transformation finale
    vec3 worldPos = lookAtRotation * localPos + orbitPos;

    v_pos = worldPos;
    gl_Position = MVP * vec4(worldPos, 1.0);
}
