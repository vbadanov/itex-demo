#ifndef CGATEPP_STATE_HPP_
#define CGATEPP_STATE_HPP_

namespace cgatepp
{

enum class State
{
    CLOSED,
    ERROR,
    OPENING,
    ACTIVE
};

inline State map_cg_state(uint32_t cg_state)
{
    State state;
    switch(cg_state)
    {
        case CG_STATE_CLOSED:
            state = State::CLOSED;
            break;

        default:
        case CG_STATE_ERROR:
            state = State::ERROR;
            break;

        case CG_STATE_OPENING:
            state = State::OPENING;
            break;

        case CG_STATE_ACTIVE:
            state = State::ACTIVE;
            break;
    }
    return state;
}


}
#endif //CGATEPP_STATE_HPP_
