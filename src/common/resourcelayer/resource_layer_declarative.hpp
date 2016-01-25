//
//  resource_layer_declarative.hpp
//  GLSandbox
//
//  Created by semery on 1/24/16.
//  Copyright © 2016 Seiji Emery. All rights reserved.
//

#ifndef resource_layer_declarative_hpp
#define resource_layer_declarative_hpp

#include "resource_impl.hpp"
#include <cassert>

namespace gl_sandbox {
namespace resource {
namespace declarative {
using namespace resource_impl;
    
class LoadAsFileBuffer {
    FilePath m_filePath;
    async::FileBufferCallback m_onLoad { nullptr };
    async::FilePathCallback   m_onFail { nullptr };

public:
    LoadAsFileBuffer (const FilePath & path) : m_filePath(path) {}
    auto & onLoaded (decltype(m_onLoad) onLoad) {
        return m_onLoad = onLoad, *this;
    }
    auto & onLoadFailed (decltype(m_onFail) onFail) {
        return m_onFail = onFail, *this;
    }
    ~LoadAsFileBuffer () {
        assert(m_onLoad || m_onFail);
        async::loadFileAsync(m_filePath, m_onLoad, m_onFail);
    }
};
    
class LoadAsCFile {
    FilePath m_filePath;
    std::string m_mode;
    async::CFileCallback m_onLoad { nullptr };
    async::CFileErrorCallback m_onFail { nullptr };
    
public:
    LoadAsCFile (const FilePath & path, const char * mode) :
        m_filePath(path), m_mode(mode) {}
    auto & onLoaded (decltype(m_onLoad) onLoad) {
        return m_onLoad = onLoad, *this;
    }
    auto & onLoadFailed (decltype(m_onFail) onFail) {
        return m_onFail = onFail, *this;
    }
    ~LoadAsCFile () {
        assert(m_onLoad || m_onFail);
        async::loadFileAsync(m_filePath, m_mode.c_str(), m_onLoad, m_onFail);
    }
};

class LoadAsIFstream {
    FilePath m_filePath;
    async::IFStreamCallback m_onLoad { nullptr };
    async::FilePathCallback m_onFail { nullptr };
    
public:
    LoadAsIFstream (const FilePath & path) : m_filePath(path) {}
    auto & onLoaded (decltype(m_onLoad) onLoad) {
        return m_onLoad = onLoad, *this;
    }
    auto & onLoadFailed (decltype(m_onFail) onFail) {
        return m_onFail = onFail, *this;
    }
    ~LoadAsIFstream () {
        assert(m_onLoad || m_onFail);
        async::loadFileAsync(m_filePath, m_onLoad, m_onFail);
    }
};
    
}; // namespace declarative
}; // namespace resource
}; // namespace gl_sandbox

#endif /* resource_layer_declarative_hpp */
