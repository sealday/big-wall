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
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            void main() {
                gl_Position = projection * view * model * vec4(position, 1.0);
        }

    );


    const char *fShader = GLSL
    (
            uniform vec3 useColor;
            out vec4 color;
            void main() {
                color = vec4(useColor, 1.0);
            }
    );
}



#endif //BIG_WALL_SHADER_STRINGS_H
