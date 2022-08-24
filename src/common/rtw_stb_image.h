//
// Created by George on 2022/8/24.
//

#ifndef RAY_TRACING_RTW_STB_IMAGE_H
#define RAY_TRACING_RTW_STB_IMAGE_H

// Disable pedantic warnings for this external library.
#ifdef _MSC_VER
// Microsoft Visual C++ Compiler
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// Restore warning levels.
#ifdef _MSC_VER
// Microsoft Visual C++ Compiler
    #pragma warning (pop)
#endif

#endif //RAY_TRACING_RTW_STB_IMAGE_H
