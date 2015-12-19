//
//  shaders.hpp
//  GLSandbox
//
//  Created by semery on 12/14/15.
//  Copyright © 2015 Seiji Emery. All rights reserved.
//

#ifndef shaders_hpp
#define shaders_hpp

#include "gl_traits.hpp"
#include "gl_error.hpp"
#include <functional>
#include <sstream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

namespace gl_sandbox {
namespace gl {

bool compileShader (gl::FragmentShader & shader, const char * src, std::function<void(const char *)> onError);
bool compileShader (gl::VertexShader & shader, const char * src, std::function<void(const char *)> onError);
bool linkShader (gl::ShaderProgram & program, const gl::VertexShader & vertexShader, const gl::FragmentShader & fragmentShader, std::function<void(const char *)> onError);

struct ShaderLoadError : public std::runtime_error {
    ShaderLoadError (const std::string & s) : std::runtime_error(s) {}
};

class Shader {
public:
    Shader (const char * name) : name(name) {}
    Shader (const std::string & name) : name(name) {}
protected:
    gl::ShaderProgram  _program;
    gl::FragmentShader _fs;
    gl::VertexShader   _vs;
public:
    std::string name;
protected:
    bool fragment_compiled = false;
    bool vertex_compiled = false;
    bool program_linked = false;
public:
    decltype(_program.handle) handle () const {
        return _program.handle;
    }
    typedef std::function<void(const ShaderLoadError &)> ErrorCallback;

    bool compileFragment (const char * src, ErrorCallback onError = dumpToStderr);
    bool compileVertex (const char * src, ErrorCallback onError = dumpToStderr);
    bool linkProgram (ErrorCallback onError = dumpToStderr);
    
    bool loaded () const {
        assert(program_linked ? fragment_compiled && vertex_compiled : true); // sanity check state flags
        return program_linked;
    }
    GLint getUniformLocation (const char * name) {
        auto loc = glGetUniformLocation(handle(), name);
        if (loc < 0)
            std::cerr << "Error: No uniform '" << name << "' in shader '" << this->name << "'\n";
        return loc;
    }
    void setUniform (GLint location, const glm::mat4x4 & m) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
    }
    void setUniform (GLint location, const glm::vec3 & v) {
        glUniform3fv(location, 1, &v[0]);
    }
    void setUniform (GLint location, const glm::vec4 & v) {
        glUniform4fv(location, 1, &v[0]);
    }
    void setUniform (GLint location, float s) {
        glUniform1f(location, s);
    }
    template <typename T>
    void setUniform (const char * name, const T & v) { setUniform(getUniformLocation(name), v); }
    
protected:
    // Default impl for the optional onError parameter on compileFragment, etc;
    // Just takes message (e.what) and prints to stdout (cerr).
    static void dumpToStderr (const ShaderLoadError & e);
};

namespace ubo_fast {
  
template <typename T>
struct UniformBuffer {
    UniformBuffer (const Shader & shader, const char * uniformBlockName) : handle(m_buffer.handle) {
        auto index = glGetUniformBlockIndex(shader.handle(), uniformBlockName);
        if (index <= 0) {
            std::cerr << "Invalid uniform block -- no block matching '" << uniformBlockName << "' in shader '" << shader.name << "'\n";
            return;
        }
        GLint blockSize;
        glGetActiveUniformBlockiv(shader.handle(), index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        if (blockSize != sizeof(T)) {
            std::cerr << "Warning: Uniform block '" << uniformBlockName << "' in shader '" << shader.name << "' size does not match cpp data structure (" << blockSize << " != " << sizeof(T) << "). This will likely cause data alignment issues, etc\n";
        }
        m_blockSize = std::min((size_t)std::max((decltype(blockSize))0, blockSize), sizeof(T));
    }
    void bufferData (const T & data) {
        glBindBuffer(GL_UNIFORM_BUFFER, handle);
        glBufferData(GL_UNIFORM_BUFFER, m_blockSize, &data, GL_DYNAMIC_DRAW);
    }
protected:
    VBO m_buffer;
public:
    const GLuint handle;
protected:
    size_t m_blockSize = 0;
};
    
}; // namespace ubo_fast
    
namespace ubo_dynamic {
    
// Dynamic / introspective UniformBuffer impl. Wraps a type (T), taking shader and field info (for each used field in T and the shader)
// at construction time, which it uses to build a list of src/dst field offsets so that information can be safely transferred to
// the shader from any arbitrary POD, even if their internal data layout does not match.
//
// Advantages: (as opposed to simply calling glBufferData w/ a pointer to a type T, which would be simpler and probably faster)
// - Stride / padding is a non-issue (ie. vec3 => padded vec4, etc)
// - Fields can be out of order (Foo, Bar, Baz => Foo, Baz, Bar) -- probably not that important, but w/e, it's idiot-proof.
// - Shader can have fields that are unused on the cpu end, and vice versa (only fields that are exposed in the constructor are used)
// - Pretty much guaranteed to work (and gives you errors when you fuck up), regardless of the shit you do to the underlying data structures
// Disadvantages:
// - Requires lots of state in the ctor (some of which could be tricky to get -- eg. shader has to be initialized, etc)
// - Fields need to be added to said ctor whenever you change any of the underlying data structures (and forgetting to do so could
// result in silent errors shader-side, etc)
// - Probably slower (or at least more overhead) than a plain call to glBufferData. The whole point of UBOs is to limit buffer
// calls though, so the perf hit is probably minimal. Should test.
//
template <typename T>
struct UniformBuffer {
    struct FieldDescriptor {
        std::string name;
        size_t fieldSize;
        void * fieldPtr;
        
        template <typename U>
        FieldDescriptor (const char * name, size_t fieldSize, U * fieldPtr) :
            FieldDescriptor(name, fieldSize, (void*)fieldPtr) {}
        FieldDescriptor (const char * name, size_t fieldSize, void * fieldPtr) :
            name(name), fieldSize(fieldSize), fieldPtr(fieldPtr) {}
    };
    struct FieldOffset {
        size_t src_offset;
        size_t dst_offset;
        size_t size;
        
        FieldOffset (size_t src_offset, size_t dst_offset, size_t size) :
            src_offset(src_offset), dst_offset(dst_offset), size(size) {}
    };
    
    UniformBuffer (const Shader & shader, const char * uniformBlockName, T * startPtr, std::initializer_list<FieldDescriptor> fields) : UniformBuffer(shader, uniformBlockName, (void*)startPtr, fields) {}
    
    UniformBuffer (const Shader & shader, const char * uniformBlockName, void * startPtr, std::initializer_list<FieldDescriptor> fields) :
        handle(m_buffer.handle),
        m_blockIndex(glGetUniformBlockIndex(shader.handle(), uniformBlockName)),
        m_blockSize(std::max(getBlockSize(shader, m_blockIndex), (GLuint)0)),
        m_block(m_blockSize > 0 ? new GLubyte(m_blockSize) : nullptr)
    {
        if (m_blockIndex <= 0) {
            std::cerr << "Invalid uniform block -- no matching block named '" << uniformBlockName << "' in shader '" << shader.name << "'\n";
            return;
        }
        GLuint fieldIndices [fields.size()];
        GLint fieldOffsets [fields.size()];
        const char * names  [fields.size()];
        auto i = 0;
        for (auto field : fields)
            names[i++] = field.name.c_str();
        
        glGetUniformIndices(shader.handle(), (int)fields.size(), names, fieldIndices);
        glGetActiveUniformsiv(shader.handle(), (int)fields.size(), fieldIndices, GL_UNIFORM_OFFSET, fieldOffsets);
        
        // Can optimize this by finding + merging fields / sections of data that are adjacent to one another (ie. no padding).
        // Since, uh, we're currently not doing that xD
        // (If a data structure is used that maps 1-1 to the gpu version (offsets, etc), it _should_ be possible to optimize
        // that down to one memcpy (and we could maybe even have it skip the memcpy and do a direct buffer-from-ptr in that
        // case. However, our current behavior (doing N memcpys) is probably not _too_ terrible since this should be called once per
        // frame at most...)

        i = 0;
        size_t totalSize = 0;
        for (auto field : fields) {
            size_t fieldOffset = (size_t)field.fieldPtr - (size_t)startPtr;
            assert(fieldOffset < sizeof(T));
            m_offsets.emplace_back(fieldOffset, fieldOffsets[i], field.fieldSize);
            totalSize += field.fieldSize;
        }
        assert(sizeof(T) >= totalSize);
    }
    void bufferData (const T & data) {
        if (m_block == nullptr) // invalid ubo
            return;
        
        uint8_t * src = (uint8_t*)&data;
        for (auto field : m_offsets)
            memcpy(m_block + field.dst_offset, src + field.src_offset, field.size);
        glBindBuffer(GL_UNIFORM_BUFFER, handle);
        glBufferData(GL_UNIFORM_BUFFER, m_blockSize, m_block, GL_DYNAMIC_DRAW);
    }
protected:
    // Helper function
    static GLuint getBlockSize (const Shader & shader, GLint blockIndex) {
        GLint blockSize; return glGetActiveUniformBlockiv(shader.handle(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize), blockSize;
    }
protected:
    VAO m_buffer; // doubles as a UBO
public:
    GLuint handle;
protected:
    const GLint m_blockIndex;
    const GLint m_blockSize;
    GLubyte * const m_block;
    std::vector<FieldOffset> m_offsets;
};
    
struct Foo {
    glm::vec4 innerColor;
    glm::vec4 outerColor;
    float radiusInner;
    float radiusOuter;
};
    
inline void example () {
    Shader shader { "some shader" };
    Foo foo;
    UniformBuffer<Foo> fooUbo { shader, "BlobSettings", &foo, {
        { "InnerColor", sizeof(foo.innerColor), &foo.innerColor },
        { "OuterColor", sizeof(foo.outerColor), &foo.outerColor },
        { "RadiusInner", sizeof(foo.radiusInner), &foo.radiusInner },
        { "RadiusOuter", sizeof(foo.radiusOuter), &foo.radiusOuter }
    } };
    fooUbo.bufferData(foo);
}
    
    
};
    
}; // namespace gl
}; // namespace gl_sandbox



#endif /* shaders_hpp */
