
require "gsb_runtime", "gl", "math"

INSTANCE_ARRAY_DIMENSIONS =
    x: 1000,
    y: 1000,
    z: 1000
SCALE_FACTOR = 5.0
ROTATION_PERIOD = 2.5

create_module "triangles", =>
    -- shader = with @require_resource "shader/common/basic_shader.gl_shader"
    shader = with @create_shader!
        \vertex [[
            #version 410

            layout (location = 0) in vec3 VertexPosition;
            layout (location = 1) in vec3 VertexColor;
            layout (location = 2) in vec3 InstancePosition;

            out vec3 Color;
            uniform mat4 ViewProjMatrix;
            uniform mat4 RotationMatrix;

            void main () {
                Color = VertexColor;
                gl_Position = ViewProjMatrix * (vec4(InstancePosition, 1.0) + (RotationMatrix * vec4(VertexPosition)));
            }
        ]]
        \fragment [[
            #version 410
            in vec3 Color;
            out vec4 FragColor;

            void main () {
                FragColor = vec4(Color, 1.0);
            }
        ]]
    shader_variables =
        vp: shader\getUniform "ViewProjMatrix"
        rotMatrix: shader\getUniform "RotationMatrix"

    triangle =
        position_data: @array "vec3f", {
            -0.8, -0.8, 0.0,
            0.8, -0.8, 0.0,
            0.0, 0.8, 0.0
        },
        color_data: @array "vec3f", {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        },
        instance_positions: @generate_array "vec3f" () ->
            for i = 1, INSTANCE_ARRAY_DIMENSIONS.x
                for j = 1, INSTANCE_ARRAY_DIMENSIONS.y
                    for k = 1, INSTANCE_ARRAY_DIMENSIONS.z
                        coroutine.yield(
                            (i - INSTANCE_ARRAY_DIMENSIONS.x * 0.5) * SCALE_FACTOR,
                            (j - INSTANCE_ARRAY_DIMENSIONS.y * 0.5) * SCALE_FACTOR,
                            (k - INSTANCE_ARRAY_DIMENSIONS.z * 0.5) * SCALE_FACTOR
                        ),
        position_buffer: with @create_buffer GL_ARRAY_BUFFER, STATIC_DRAW
            \bufferData triangle.position_data,
        color_buffer: with @create_buffer GL_ARRAY_BUFFER, STATIC_DRAW
            \bufferData triangle.color_data,
        instance_buffer: with @create_instance_buffer GL_ARRAY_BUFFER, STATIC_DRAW
            \setVertexDivisor 1
            \bufferData triangle.instance_positions,

        vao = with @create_vao!
            \bindArrayBuffer 0, 3, position_buffer
            \bindArrayBuffer 1, 3, color_buffer
            \bindArrayBuffer 2, 3, instance_buffer


    -- create a timer that is persistent (automatically serialized / deserialized),
    -- and operates in graphics (displayed) time (ie. does not advance while scripts
    -- are being loaded/unloaded, etc)
    timer = @create_local_persistent_timer USING_GRAPHICS_TIME

    two_pi = math.pi * 2
    rotation_speed = two_pi / ROTATION_PERIOD

    @on_load =>
        @console.log("Loaded lua triangles module")
        timer\start()

    @gl_draw =>
        shader\useProgram()
        triangle.vao\bind()

        angle = fmod(rotation_speed * timer\elapsed_time_since_application_start(), two_pi)

        shader_variables.vp.set(@camera.view_projection_matrix())
        shader_variables.rotMatrix.set(
            matrix.rotate(mat4.identity!, angle, vec3(0, 1, 0))
        )
        @glDrawArraysInstanced(GL_TRIANGLES, 0, 3, instance_positions\size!)

    @persist_state =>
        @use_gkey @MODULE_NAME .. '/persistent_data/'
        @store_load timer 'time' PERSIST_THROUGH_RELOADS_YES PERSIST_THOUGH_APP_LAUNCHES_NO

