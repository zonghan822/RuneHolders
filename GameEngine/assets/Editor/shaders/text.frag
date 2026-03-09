#version 450 core
    
in vec4 v_color;
in vec2 v_texCoords;

out vec4 FragColor;

uniform sampler2D u_text;

void main() {
    float alpha = texture(u_text, v_texCoords).r;
    FragColor = vec4(v_color.rgb, alpha * v_color.a);
}