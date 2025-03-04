#version 330 core

in vec3 fragNormal;    // ���ߣ��Ӷ�����ɫ������
in vec3 fragPosition;  // Ƭ����������

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform PointLight pointLight;
uniform Material material;
uniform DirectionalLight dirLight;
uniform vec3 CameraPos; // �۲���λ��

out vec4 fragColor;

void main()
{
    // ��һ������
    vec3 norm = normalize(fragNormal);
    if (!gl_FrontFacing) {
        norm = -norm;
    } 

    vec3 viewDir = normalize(CameraPos - fragPosition);
    
    // ==============================
    // Directional Light ����
    // ==============================
    vec3 lightDir = normalize(dirLight.direction);

    // ������
    vec3 ambient = material.ambient * material.diffuse;

    // ������
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * dirLight.color * material.diffuse;

    // �߹� (Blinn-Phong)
    vec3 halfDir = normalize(viewDir + lightDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), material.shininess);
    vec3 specular = spec * material.specular * dirLight.color;

    vec3 dirLightColor = ambient + diffuse;
    if (diff > 0) {
        dirLightColor += specular;
    }

    // ==============================
    // Point Light ����
    // ==============================
    vec3 pointLightDir = normalize(pointLight.position - fragPosition);
    float r = length(pointLight.position - fragPosition);
    float r2 = r * r;
    // ������
    float pointDiff = max(dot(norm, pointLightDir), 0.0);
    vec3 pointDiffuse = pointDiff * pointLight.color * material.diffuse * pointLight.intensity / r2;

    // �߹� (Blinn-Phong)
    vec3 pointHalfDir = normalize(viewDir + pointLightDir);
    float pointSpec = pow(max(dot(norm, pointHalfDir), 0.0), material.shininess);
    vec3 pointSpecular = pointSpec * material.specular * pointLight.color * pointLight.intensity / r2;

    vec3 pointLightColor = pointDiffuse;
    if (pointDiff > 0) {
        pointLightColor += pointSpecular;
    }

    // ==============================
    // �ϲ����� (Directional + Point)
    // ==============================
    vec3 finalColor = dirLightColor + pointLightColor;

    fragColor = vec4(finalColor, 1.0);
}
