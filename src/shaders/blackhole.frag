#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 blackHolePos;
uniform float eventHorizonRadius;
uniform float time;

// Функция шума для эффектов
float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 421.9))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    return mix(
        mix(
            mix(hash(i + vec3(0.0, 0.0, 0.0)), hash(i + vec3(1.0, 0.0, 0.0)), f.x),
            mix(hash(i + vec3(0.0, 1.0, 0.0)), hash(i + vec3(1.0, 1.0, 0.0)), f.x),
            f.y
        ),
        mix(
            mix(hash(i + vec3(0.0, 0.0, 1.0)), hash(i + vec3(1.0, 0.0, 1.0)), f.x),
            mix(hash(i + vec3(0.0, 1.0, 1.0)), hash(i + vec3(1.0, 1.0, 1.0)), f.x),
            f.y
        ),
        f.z
    );
}

void main() {
    vec3 toBlackHole = blackHolePos - FragPos;
    float distance = length(toBlackHole);
    
    // Основные зоны черной дыры
    if (distance < eventHorizonRadius * 1.02) {
        // Внутри горизонта событий - абсолютная чернота
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        
    } else if (distance < eventHorizonRadius * 1.5) {
        // Непосредственно у горизонта событий - красное свечение
        float intensity = (distance - eventHorizonRadius * 1.02) / (eventHorizonRadius * 0.48);
        intensity = pow(intensity, 0.5);
        
        // Пульсирующий эффект
        float pulse = sin(time * 5.0) * 0.1 + 0.9;
        vec3 color = mix(vec3(0.3, 0.0, 0.0), vec3(0.8, 0.1, 0.0), intensity) * pulse;
        
        FragColor = vec4(color, 1.0);
        
    } else if (distance < eventHorizonRadius * 3.0) {
        // Область сильного гравитационного линзирования
        float intensity = (distance - eventHorizonRadius * 1.5) / (eventHorizonRadius * 1.5);
        
        // Эффект искажения пространства-времени
        vec3 warpedPos = FragPos + normalize(toBlackHole) * sin(time * 3.0 + distance) * 0.1;
        float warpNoise = noise(warpedPos * 2.0 + time);
        
        vec3 color = mix(vec3(0.4, 0.15, 0.05), vec3(0.2, 0.05, 0.0), intensity);
        color += warpNoise * 0.1;
        
        FragColor = vec4(color, 0.9);
        
    } else {
        // Слабое гравитационное влияние
        float influence = eventHorizonRadius / distance;
        influence = clamp(influence, 0.0, 0.3);
        
        // Едва заметное искажение
        float subtleWarp = sin(time * 0.5 + distance * 0.5) * 0.05;
        vec3 color = mix(vec3(0.1, 0.03, 0.0), vec3(0.05, 0.01, 0.0), influence);
        color += vec3(subtleWarp * 0.1);
        
        FragColor = vec4(color, 0.7);
    }
    
    // Добавляем эффект гравитационного линзирования на краях
    float lensingEdge = 1.0 - smoothstep(eventHorizonRadius * 2.0, eventHorizonRadius * 4.0, distance);
    if (lensingEdge > 0.0) {
        vec3 lensingColor = vec3(0.3, 0.2, 0.1) * lensingEdge * 0.3;
        FragColor.rgb += lensingColor;
    }
}
