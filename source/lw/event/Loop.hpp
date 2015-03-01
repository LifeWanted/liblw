#pragma once

struct uv_loop_s;

namespace lw {
namespace event {

class Loop {
public:
    Loop( void );

    Loop( const Loop& ) = delete;

    Loop( Loop&& other ):
        m_loop( other.m_loop )
    {}

    virtual ~Loop( void );

    void run( void );

    uv_loop_s* lowest_layer( void ){
        return m_loop;
    }

private:
    uv_loop_s* m_loop;
};

}
}
