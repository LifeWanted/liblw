#pragma once

#define LW_EXPAND(x) x
#define LW_CONCAT(x,y) x##y
#define LW_STRINGIFY_IMPL(x) #x
#define LW_STRINGIFY(x) LW_STRINGIFY_IMPL(x)

#define _LW_FOR_EACH_1(op, x, ...) op(x)
#define _LW_FOR_EACH_2(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_1(op, __VA_ARGS__))
#define _LW_FOR_EACH_3(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_2(op, __VA_ARGS__))
#define _LW_FOR_EACH_4(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_3(op, __VA_ARGS__))
#define _LW_FOR_EACH_5(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_4(op, __VA_ARGS__))
#define _LW_FOR_EACH_6(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_5(op, __VA_ARGS__))
#define _LW_FOR_EACH_7(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_6(op, __VA_ARGS__))
#define _LW_FOR_EACH_8(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_7(op, __VA_ARGS__))
#define _LW_FOR_EACH_9(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_8(op, __VA_ARGS__))
#define _LW_FOR_EACH_10(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_9(op, __VA_ARGS__))
#define _LW_FOR_EACH_11(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_10(op, __VA_ARGS__))
#define _LW_FOR_EACH_12(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_11(op, __VA_ARGS__))
#define _LW_FOR_EACH_13(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_12(op, __VA_ARGS__))
#define _LW_FOR_EACH_14(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_13(op, __VA_ARGS__))
#define _LW_FOR_EACH_15(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_14(op, __VA_ARGS__))
#define _LW_FOR_EACH_16(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_15(op, __VA_ARGS__))
#define _LW_FOR_EACH_17(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_16(op, __VA_ARGS__))
#define _LW_FOR_EACH_18(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_17(op, __VA_ARGS__))
#define _LW_FOR_EACH_19(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_18(op, __VA_ARGS__))
#define _LW_FOR_EACH_20(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_19(op, __VA_ARGS__))
#define _LW_FOR_EACH_21(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_20(op, __VA_ARGS__))
#define _LW_FOR_EACH_22(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_21(op, __VA_ARGS__))
#define _LW_FOR_EACH_23(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_22(op, __VA_ARGS__))
#define _LW_FOR_EACH_24(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_23(op, __VA_ARGS__))
#define _LW_FOR_EACH_25(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_24(op, __VA_ARGS__))
#define _LW_FOR_EACH_26(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_25(op, __VA_ARGS__))
#define _LW_FOR_EACH_27(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_26(op, __VA_ARGS__))
#define _LW_FOR_EACH_28(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_27(op, __VA_ARGS__))
#define _LW_FOR_EACH_29(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_28(op, __VA_ARGS__))
#define _LW_FOR_EACH_30(op, x, ...) op(x); LW_EXPAND(_LW_FOR_EACH_29(op, __VA_ARGS__))

#define _LW_FOR_EACH_ARG_N(                             \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,            \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,   \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,   \
    N, ...) N
#define _LW_FOR_EACH_RSEQ_N()               \
    30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
    20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
    10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define _LW_FOR_EACH_NARG_IMPL(...) LW_EXPAND(_LW_FOR_EACH_ARG_N(__VA_ARGS__))
#define _LW_FOR_EACH_NARG(...) _LW_FOR_EACH_NARG_IMPL(__VA_ARGS__, _LW_FOR_EACH_RSEQ_N())

#define _LW_FOR_EACH_IMPL(N, op, ...) LW_EXPAND(LW_CONCAT(_LW_FOR_EACH_, N)(op, __VA_ARGS__))

#define LW_FOR_EACH(op, ...) _LW_FOR_EACH_IMPL(_LW_FOR_EACH_NARG(__VA_ARGS__), op, __VA_ARGS__)
