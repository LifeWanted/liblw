
#include "lw/Application.hpp"

namespace lw {

LW_DEFINE_SINGLETON_INSTANCE( Application );

Application::Application( void ):
    Loop(),
    Singleton< Application >()
{}

Application::~Application( void ){}

}
