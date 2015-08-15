#pragma once

#include "lw/event/Loop.hpp"
#include "lw/Singleton.hpp"

namespace lw {

class Application : public event::Loop, public Singleton<Application>{
public:
    Application(void);
    ~Application(void);
};

}
