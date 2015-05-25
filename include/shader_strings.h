//
// Created by seal on 15-5-25.
//

#ifndef BIG_WALL_SHADER_STRINGS_H
#define BIG_WALL_SHADER_STRINGS_H

#define GLSL(src) "#version 330 core\n"#src

namespace glsl {

    const char *vShader = GLSL
    (
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec2 texCoord;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            out vec2 TexCoord;
            void main() {
                gl_Position = projection * view * model * vec4(position, 1.0);
                TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
            }

    );


    const char *fShader = GLSL
    (
            in vec2 TexCoord;
            uniform vec3 useColor;
            out vec4 color;

            uniform sampler2D texture1;
            void main() {
                color = texture(texture1, TexCoord);
            }
    );
}



#endif //BIG_WALL_SHADER_STRINGS_H
