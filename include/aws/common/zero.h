#ifndef AWS_COMMON_ZERO_H
#define AWS_COMMON_ZERO_H

/*
 * Copyright 2010-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws/common/stdbool.h>
#include <aws/common/stdint.h>

#include <string.h>

AWS_EXTERN_C_BEGIN

/**
 * Set each byte in the struct to zero.
 */
#define AWS_ZERO_STRUCT(object)                                                                                        \
    do {                                                                                                               \
        memset(&(object), 0, sizeof(object));                                                                          \
    } while (0)

/**
 * Set each byte in the array to zero.
 * Does not work with arrays of unknown bound.
 */
#define AWS_ZERO_ARRAY(array) memset((void *)(array), 0, sizeof(array))

/**
 * Returns whether each byte in the object is zero.
 */
#if defined(CBMC) && !defined(CBMC_NO_FORALL)
/* clang-format off */
#    define AWS_IS_ZEROED(object)                                                                                      \
        __CPROVER_forall {                                                                                             \
            int i;                                                                                                     \
            (i >= 0 && i < sizeof(object)) ==> ((const uint8_t *)&object)[i] == 0                                      \
        }
/* clang-format on */
#else
#    define AWS_IS_ZEROED(object) aws_is_mem_zeroed(&(object), sizeof(object))
#endif

/**
 * Returns whether each byte is zero.
 */
AWS_STATIC_IMPL
bool aws_is_mem_zeroed(const void *buf, size_t bufsize);

/**
 * Securely zeroes a memory buffer. This function will attempt to ensure that
 * the compiler will not optimize away this zeroing operation.
 */
AWS_COMMON_API
void aws_secure_zero(void *pBuf, size_t bufsize);

#ifndef AWS_NO_STATIC_IMPL
#    include <aws/common/zero.inl>
#endif /* AWS_NO_STATIC_IMPL */

AWS_EXTERN_C_END

#endif /* AWS_COMMON_ZERO_H */
