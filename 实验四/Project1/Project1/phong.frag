#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;   // 指向“光照射方向”（世界空间）
    vec3 color;
    float intensity;
};

struct SpotLight {
    vec3 position;
    vec3 direction;   // 聚光灯朝向（世界空间）
    vec3 color;
    float intensity;
    float innerCos;
    float outerCos;
    float range;
    int enabled;
};

in VS_OUT {
    vec3 worldPos;
    vec3 normalW;
    vec2 uv;
} fs_in;

out vec4 FragColor;

uniform vec3 uViewPos;

uniform Material uMat;
uniform DirLight  uSun;
uniform SpotLight uSpot;
uniform int  uUnlit;
uniform vec3 uUnlitColor;

uniform int  uUseTexture;
uniform sampler2D uTexture;

uniform int   uFogEnabled;
uniform int   uFogMode;      // 0 linear, 1 exp, 2 exp2
uniform vec3  uFogColor;
uniform float uFogDensity;
uniform float uFogStart;
uniform float uFogEnd;

vec3 applyDirLight(DirLight L, vec3 N, vec3 V, vec3 albedo) {
    vec3 lightDir = normalize(-L.direction);
    float ndl = max(dot(N, lightDir), 0.0);

    vec3 H = normalize(lightDir + V);
    float spec = pow(max(dot(N, H), 0.0), uMat.shininess);

    vec3 ambient  = uMat.ambient * albedo;
    vec3 diffuse  = uMat.diffuse * albedo * ndl;
    vec3 specular = uMat.specular * spec;

    return (ambient + diffuse + specular) * L.color * L.intensity;
}

vec3 applySpotLight(SpotLight L, vec3 N, vec3 V, vec3 albedo) {
    if (L.enabled == 0) return vec3(0.0);

    vec3 toFrag = fs_in.worldPos - L.position;
    float dist = length(toFrag);
    if (dist > L.range) return vec3(0.0);

    vec3 lightDir = normalize(-toFrag);        // frag <- light
    vec3 spotDir  = normalize(L.direction);    // light forward

    float cosTheta = dot(normalize(toFrag), spotDir); // toFrag 与 spotDir 同向时最大
    float cone = smoothstep(L.outerCos, L.innerCos, cosTheta);

    float att = clamp(1.0 - dist / L.range, 0.0, 1.0);

    float ndl = max(dot(N, lightDir), 0.0);
    vec3 H = normalize(lightDir + V);
    float spec = pow(max(dot(N, H), 0.0), uMat.shininess);

    vec3 ambient  = uMat.ambient * albedo;
    vec3 diffuse  = uMat.diffuse * albedo * ndl;
    vec3 specular = uMat.specular * spec;

    return (ambient + diffuse + specular) * L.color * (L.intensity * cone * att);
}

float fogFactor(float d) {
    if (uFogEnabled == 0) return 1.0;

    if (uFogMode == 0) { // linear
        float f = (uFogEnd - d) / (uFogEnd - uFogStart);
        return clamp(f, 0.0, 1.0);
    } else if (uFogMode == 1) { // exp
        return exp(-uFogDensity * d);
    } else { // exp2
        float x = uFogDensity * d;
        return exp(-(x * x));
    }
}

void main() {
    if (uUnlit == 1) {
        FragColor = vec4(uUnlitColor, 1.0);
        return;
    }

    vec3 N = normalize(fs_in.normalW);
    vec3 V = normalize(uViewPos - fs_in.worldPos);

    vec3 albedo = uMat.diffuse;
    if (uUseTexture != 0) {
        albedo *= texture(uTexture, fs_in.uv).rgb;
    }

    vec3 color = vec3(0.0);
    color += applyDirLight(uSun, N, V, albedo);
    color += applySpotLight(uSpot, N, V, albedo);

    float d = length(uViewPos - fs_in.worldPos);
    float f = fogFactor(d);
    color = mix(uFogColor, color, f);

    FragColor = vec4(color, 1.0);
}
