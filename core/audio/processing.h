#pragma once
#include "sample_arithmetic.h"


namespace Beatmup {
    namespace AudioProcessing {

        template<template<typename, typename> class Func, typename... Args>
        inline void pipeline(const AudioSampleFormat inFormat, const AudioSampleFormat outFormat, const psample* input, psample* output, Args&&... args) {

#define WRITING(IN_T)  \
            switch (outFormat) { \
                case Int8: \
                    Func<IN_T, sample8>::process((const IN_T*)input, (sample8*)output, args...); \
                    break; \
                case Int16: \
                    Func<IN_T, sample16>::process((const IN_T*)input, (sample16*)output, args...); \
                    break; \
                case Int32: \
                    Func<IN_T, sample32>::process((const IN_T*)input, (sample32*)output, args...); \
                    break; \
                case Float32: \
                    Func<IN_T, sample32f>::process((const IN_T*)input, (sample32f*)output, args...); \
                    break; \
            }

            switch (inFormat) {
            case Int8:
                WRITING(sample8);
                break;
            case Int16:
                WRITING(sample16);
                break;
            case Int32:
                WRITING(sample32);
                break;
            case Float32:
                WRITING(sample32f);
                break;
            }
#undef WRITING
        }
    }
}
