/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*! \file general_copy.h
 *  \brief Sequential copy algorithms for general iterators.
 */

#pragma once

#include <thrust/detail/config.h>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header
#include <thrust/detail/raw_reference_cast.h>
#include <thrust/detail/type_traits.h>

THRUST_NAMESPACE_BEGIN
namespace system
{
namespace detail
{
namespace sequential
{
namespace general_copy_detail
{

template <typename T1, typename T2>
struct lazy_is_assignable : ::cuda::std::is_assignable<typename T1::type, typename T2::type>
{};

// sometimes OutputIterator's reference type is reported as void
// in that case, just assume that we're able to assign to it OK
template <typename InputIterator, typename OutputIterator>
struct reference_is_assignable
    : thrust::detail::eval_if<
        ::cuda::std::is_same<typename thrust::iterator_reference<OutputIterator>::type, void>::value,
        thrust::detail::true_type,
        lazy_is_assignable<thrust::iterator_reference<OutputIterator>, thrust::iterator_reference<InputIterator>>>::type
{};

// introduce an iterator assign helper to deal with assignments from
// a wrapped reference

_CCCL_EXEC_CHECK_DISABLE
template <typename OutputIterator, typename InputIterator>
inline _CCCL_HOST_DEVICE ::cuda::std::enable_if_t<reference_is_assignable<InputIterator, OutputIterator>::value>
iter_assign(OutputIterator dst, InputIterator src)
{
  *dst = *src;
}

_CCCL_EXEC_CHECK_DISABLE
template <typename OutputIterator, typename InputIterator>
inline _CCCL_HOST_DEVICE
typename thrust::detail::disable_if<reference_is_assignable<InputIterator, OutputIterator>::value>::type
iter_assign(OutputIterator dst, InputIterator src)
{
  using value_type = typename thrust::iterator_value<InputIterator>::type;

  // insert a temporary and hope for the best
  *dst = static_cast<value_type>(*src);
}

} // namespace general_copy_detail

_CCCL_EXEC_CHECK_DISABLE
template <typename InputIterator, typename OutputIterator>
_CCCL_HOST_DEVICE OutputIterator general_copy(InputIterator first, InputIterator last, OutputIterator result)
{
  for (; first != last; ++first, (void) ++result)
  {
    // gcc 4.2 crashes while instantiating iter_assign
#if defined(_CCCL_COMPILER_GCC) && (THRUST_GCC_VERSION < 40300)
    *result = *first;
#else
    general_copy_detail::iter_assign(result, first);
#endif
  }

  return result;
} // end general_copy()

_CCCL_EXEC_CHECK_DISABLE
template <typename InputIterator, typename Size, typename OutputIterator>
_CCCL_HOST_DEVICE OutputIterator general_copy_n(InputIterator first, Size n, OutputIterator result)
{
  for (; n > Size(0); ++first, (void) ++result, (void) --n)
  {
    // gcc 4.2 crashes while instantiating iter_assign
#if defined(_CCCL_COMPILER_GCC) && (THRUST_GCC_VERSION < 40300)
    *result = *first;
#else
    general_copy_detail::iter_assign(result, first);
#endif
  }

  return result;
} // end general_copy_n()

} // end namespace sequential
} // end namespace detail
} // end namespace system
THRUST_NAMESPACE_END
