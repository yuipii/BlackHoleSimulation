#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in float DiskRadius;

uniform vec3 blackHolePos;
uniform float innerRadius;
uniform float outerRadius;
uniform float time;

// Шум функция для турбулентности
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

void main() {
    // Нормализованный радиус диска
    float t = (DiskRadius - innerRadius) / (outerRadius - innerRadius);
    t = clamp(t, 0.0, 1.0);
    
    // Температурный градиент - горячее ближе к черной дыре
    float temperature = 1.0 - t;
    
    // Угловая координата для текстуры
    float angle = atan(WorldPos.z, WorldPos.x);
    vec2 uv = vec2(angle / (2.0 * 3.14159), DiskRadius / outerRadius);
    
    // Турбулентность и завихрения
    float turbulence = fbm(uv * 8.0 + time * 0.5) * 0.4;
    turbulence += fbm(uv * 16.0 - time * 0.8) * 0.2;
    
    // Эффект Доплера - синее смещение на приближающейся стороне, красное на удаляющейся
    vec3 viewDir = normalize(-WorldPos);
    vec3 diskNormal = vec3(0.0, 1.0, 0.0);
    float doppler = dot(diskNormal, viewDir);
    
    // Вращение диска (эффект увлечения инерциальных систем отсчета)
    float frameDragging = sin(DiskRadius * 0.3 - time * 2.0) * 0.1;
    
    // Цветовая схема на основе температуры (как в Интерстеллар)
    vec3 color;
    
    if (temperature > 0.8) {
        // Внутренняя горячая область - бело-голубая
        float hotness = (temperature - 0.8) * 5.0;
        color = mix(vec3(0.5, 0.7, 1.0), vec3(1.0, 1.0, 1.2), hotness);
        
        // Добавляем рентгеновское свечение
        float xray = sin(time * 3.0 + DiskRadius * 5.0) * 0.1 + 0.9;
        color *= xray;
        
    } else if (temperature > 0.5) {
        // Средняя область - желто-оранжевая
        float midness = (temperature - 0.5) * 3.33;
        color = mix(vec3(1.0, 0.8, 0.2), vec3(1.0, 1.0, 0.5), midness);
        
    } else if (temperature > 0.2) {
        // Внешняя прохладная область - оранжево-красная
        float coolness = (temperature - 0.2) * 3.33;
        color = mix(vec3(1.0, 0.3, 0.0), vec3(1.0, 0.8, 0.2), coolness);
        
    } else {
        // Самая внешняя область - темно-красная
        color = mix(vec3(0.3, 0.0, 0.0), vec3(1.0, 0.3, 0.0), temperature * 5.0);
    }
    
    // Применение эффекта Доплера
    if (doppler > 0.0) {
        // Синее смещение на приближающейся стороне
        color = mix(color, vec3(0.3, 0.5, 1.0), doppler * 0.6);
        color *= 1.0 + doppler * 0.3; // Увеличение яркости
    } else {
        // Красное смещение на удаляющейся стороне
        color = mix(color, vec3(1.0, 0.2, 0.1), -doppler * 0.6);
        color *= 1.0 - doppler * 0.2; // Уменьшение яркости
    }
    
    // Добавление турбулентности
    vec3 turbulenceColor = turbulence * vec3(0.5, 0.3, 0.1);
    color += turbulenceColor;
    
    // Яркие горячие точки (зоны повышенной активности)
    float hotspots = fbm(uv * 12.0 + time * 1.5);
    hotspots = smoothstep(0.7, 0.9, hotspots);
    color += hotspots * vec3(1.0, 0.9, 0.5) * 0.3;
    
    // Эффект релятивистских струй (джетов)
    float jetEffect = sin(angle * 8.0 + time * 4.0) * 0.5 + 0.5;
    jetEffect = pow(jetEffect, 8.0) * 0.3;
    color += jetEffect * vec3(0.8, 0.9, 1.0);
    
    // Ограничение цвета
    color = clamp(color, 0.0, 2.0);
    
    // Альфа-канал (прозрачность)
    float alpha;
    
    // Внутренний край - резкий обрыв
    if (DiskRadius < innerRadius * 1.1) {
        alpha = smoothstep(innerRadius * 0.9, innerRadius * 1.1, DiskRadius);
    } else {
        // Внешний край - плавное затухание
        alpha = 1.0 - smoothstep(outerRadius * 0.7, outerRadius, DiskRadius);
    }
    
    // Учет турбулентности в прозрачности
    alpha *= 0.8 + turbulence * 0.4;
    
    // Добавление изменчивости яркости
    float brightnessVariation = sin(time * 1.2 + DiskRadius * 0.5) * 0.1 + 0.9;
    color *= brightnessVariation;
    
    // Эффект гравитационного линзирования на внутреннем краю
    float lensing = 1.0 - smoothstep(innerRadius * 1.0, innerRadius * 1.5, DiskRadius);
    color += lensing * vec3(0.8, 0.6, 0.3) * 0.5;
    
    // Финальная коррекция цвета
    color = pow(color, vec3(1.0/2.2)); // Гамма-коррекция
    
    // Убедимся, что альфа не слишком мала
    alpha = clamp(alpha, 0.1, 1.0);
    
    FragColor = vec4(color, alpha);
    
    // Отладочная визуализация (раскомментировать при необходимости)
    /*
    if (DiskRadius < innerRadius) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Красный - внутри внутреннего радиуса
    } else if (DiskRadius > outerRadius) {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Синий - за пределами внешнего радиуса
    }
    */
}
