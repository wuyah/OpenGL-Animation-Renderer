#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in ivec4 jointIndices; // 使用4个int8
layout(location = 3) in vec3 weights;       // 自动标准化到[0,1]

uniform mat4 viewProj;
uniform mat4 model;

out vec3 fragNormal;
out vec3 fragPosition;

uniform mat4 jointMatrices[150]; 

uniform bool useGPUSkinning;     

void main() {
    if(useGPUSkinning) {
        mat4 skinMatrix = mat4(0.0);
        float totalWeight = 0.0;
    
        for(int i = 0; i < 4; ++i) {
            if(jointIndices[i] == 0xFF || 
               jointIndices[i] >= 150) 
                break;
        
            float weight = (i == 3) ? 
                (1.0 - (weights.x + weights.y + weights.z)) : 
                weights[i];
            
            skinMatrix += jointMatrices[jointIndices[i]] * weight;
            totalWeight += weight;
        }
    
        if(totalWeight > 0.0) {
            skinMatrix /= totalWeight;
        }

        vec4 skinnedPos = skinMatrix * vec4(position, 1.0);
        vec3 skinnedNormal = mat3(transpose(inverse(skinMatrix))) * normal;

        vec4 worldPos = model * skinnedPos;
        gl_Position = viewProj * worldPos;
    
        fragPosition = vec3(worldPos);
        fragNormal = normalize(mat3(transpose(inverse(model))) * skinnedNormal);
    } else {
        vec4 worldPos = model * vec4(position, 1.0);
        gl_Position = viewProj * worldPos;
        fragPosition = vec3(worldPos);
        fragNormal = normalize(mat3(transpose(inverse(model))) * normal);
    }
}
