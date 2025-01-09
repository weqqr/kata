#include <kata/resource/shader.hpp>


namespace kata {
Result<ShaderCompiler> ShaderCompiler::create()
{
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    createGlobalSession(globalSession.writeRef());

    return ShaderCompiler(globalSession);
}

void ShaderCompiler::compile(std::string_view path)
{
    slang::SessionDesc sessionDesc;
    Slang::ComPtr<slang::ISession> session;
    m_globalSession->createSession(sessionDesc, session.writeRef());

    //
}
}
