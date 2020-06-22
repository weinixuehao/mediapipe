#include "mediapipe/gpu/gl_simple_calculator.h"
#include "mediapipe/gpu/gl_simple_shaders.h"
#include "mediapipe/gpu/shader_util.h"

namespace mediapipe
{
    class HedMapRenderCalculator : public GlSimpleCalculator
    {
    public:
        ::mediapipe::Status GlSetup() override;
        ::mediapipe::Status GlRender(const GlTexture &src,
                                     const GlTexture &dst) override;
        ::mediapipe::Status GlTeardown() override;

    private:
        GLuint program_ = 0;
    };
    REGISTER_CALCULATOR(HedMapRenderCalculator);

    ::mediapipe::Status HedMapRenderCalculator::GlSetup()
    {
        const char *vert_src = 
        R"(// This matrix member variable provides a hook to manipulate
            // the coordinates of the objects that use this vertex shader
            //uniform mat4 uMVPMatrix;
            uniform mat4 rotation;
            attribute vec4 vPosition;
            void main() {
                // the matrix must be included as a modifier of gl_Position
                gl_Position = rotation * vPosition;
            })";

        const char *frag_src =
            R"(precision mediump float;
             uniform vec4 vColor;
             void main() {
                gl_FragColor = vColor;
            })";
                // shader program
        GlhCreateProgram(vert_src, frag_src, 0, nullptr, nullptr, &program_);
    }
    ::mediapipe::Status HedMapRenderCalculator::GlRender(const GlTexture &src,
                                                         const GlTexture &dst)
    {
    }
    ::mediapipe::Status HedMapRenderCalculator::GlTeardown()
    {
    }
} // namespace mediapipe