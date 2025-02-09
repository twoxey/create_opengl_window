#define MULTI_(...) #__VA_ARGS__
#define MULTI(...) MULTI_(__VA_ARGS__)

#define QUAD_rect    0
#define QUAD_ellipse 1
#define QUAD_image   2
#define QUAD_char    3

#define UNIFORMS /* Generate uniform bindings
*/ Uniform(vec2, u_screen_size) /*
*/ Uniform(vec2, u_size) /*
*/ Uniform(vec2, u_offset) /*
*/ Uniform(vec4, u_color) /*
*/ Uniform(int, u_type)

#define Uniform(type, name) static GLuint name = 0;
UNIFORMS
#undef Uniform

static GLuint quad_shader_program = 0;

void init_quad_shader() {
    quad_shader_program = create_shader_program(
        // header
        "#version 330\n"
#define Uniform(type, name) uniform type name;
        MULTI(UNIFORMS)
#undef Uniform
        ,
        // vertex shader
        MULTI(
            out vec2 frag_uv;

            void main() {
                vec2 uv = vec2(gl_VertexID & 1, (gl_VertexID >> 1) & 1);
                vec2 pos = uv;
                pos *= u_size;
                pos += u_offset;
                pos *= vec2(2) / u_screen_size;
                gl_Position = vec4(pos, 0, 1);
                frag_uv = uv;
            }
        ),
        // fragment shader
        MULTI(
            in vec2 frag_uv;
            out vec4 out_color;

            void main() {
                out_color = u_color;
                switch (u_type) {
                    case QUAD_ellipse:
                    if (length(frag_uv - vec2(.5)) > .5) {
                        out_color.a = 0;
                    }
                    break;
                }
            }
        )
    );

#define Uniform(type, name) name = glGetUniformLocation(quad_shader_program, #name);
    UNIFORMS
#undef Uniform
}

void draw_quad(int type, float x, float y, float w, float h, color_v4 c) {
    glUniform1i(u_type, type);
    glUniform2f(u_offset, x, y);
    glUniform2f(u_size, w, h);
    glUniform4f(u_color, c.r, c.g, c.b, c.a);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void draw_ellipse(float x, float y, float w, float h, color_v4 c) {
    draw_quad(QUAD_ellipse, x, y, w, h, c);
}
