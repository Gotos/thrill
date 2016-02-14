/*******************************************************************************
 * thrill/io/request_operations.hpp
 *
 * Copied and modified from STXXL https://github.com/stxxl/stxxl, which is
 * distributed under the Boost Software License, Version 1.0.
 *
 * Part of Project Thrill - http://project-thrill.org
 *
 * Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 * Copyright (C) 2008, 2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 * Copyright (C) 2009 Johannes Singler <singler@ira.uka.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once
#ifndef THRILL_IO_REQUEST_OPERATIONS_HEADER
#define THRILL_IO_REQUEST_OPERATIONS_HEADER

#include <thrill/common/onoff_switch.hpp>
#include <thrill/io/iostats.hpp>
#include <thrill/io/request.hpp>

namespace thrill {
namespace io {

//! \addtogroup reqlayer
//! \{

//! Collection of functions to track statuses of a number of requests.

//! Suspends calling thread until \b all given requests are completed.
//! \param reqs_begin begin of request sequence to wait for
//! \param reqs_end end of request sequence to wait for
template <class RequestIterator>
void wait_all(RequestIterator reqs_begin, RequestIterator reqs_end) {
    for ( ; reqs_begin != reqs_end; ++reqs_begin)
        (RequestPtr(*reqs_begin))->wait();
}

//! Suspends calling thread until \b all given requests are completed.
//! \param req_array array of request_ptr objects
//! \param count size of req_array
static inline void wait_all(RequestPtr req_array[], size_t count) {
    wait_all(req_array, req_array + count);
}

//! Cancel requests.
//! The specified requests are canceled unless already being processed.
//! However, cancelation cannot be guaranteed.
//! Cancelled requests must still be waited for in order to ensure correct
//! operation.
//! \param reqs_begin begin of request sequence
//! \param reqs_end end of request sequence
//! \return number of request canceled
template <class RequestIterator>
typename std::iterator_traits<RequestIterator>::difference_type
cancel_all(RequestIterator reqs_begin, RequestIterator reqs_end) {
    typename std::iterator_traits<RequestIterator>::difference_type num_canceled = 0;
    while (reqs_begin != reqs_end)
    {
        if ((RequestPtr(*reqs_begin))->cancel())
            ++num_canceled;
        ++reqs_begin;
    }
    return num_canceled;
}

//! Polls requests.
//! \param reqs_begin begin of request sequence to poll
//! \param reqs_end end of request sequence to poll
//! \return \c true if any of requests is completed, then index contains valid value, otherwise \c false
template <class RequestIterator>
RequestIterator poll_any(RequestIterator reqs_begin, RequestIterator reqs_end) {
    while (reqs_begin != reqs_end)
    {
        if ((RequestPtr(*reqs_begin))->poll())
            return reqs_begin;

        ++reqs_begin;
    }
    return reqs_end;
}

//! Polls requests.
//! \param req_array array of request_ptr objects
//! \param count size of req_array
//! \param index contains index of the \b first completed request if any
//! \return \c true if any of requests is completed, then index contains valid value, otherwise \c false
inline bool poll_any(RequestPtr req_array[], size_t count, size_t& index) {
    RequestPtr* res = poll_any(req_array, req_array + count);
    index = res - req_array;
    return res != (req_array + count);
}

//! Suspends calling thread until \b any of requests is completed.
//! \param reqs_begin begin of request sequence to wait for
//! \param reqs_end end of request sequence to wait for
//! \return index in req_array pointing to the \b first completed request
template <class RequestIterator>
RequestIterator wait_any(RequestIterator reqs_begin, RequestIterator reqs_end) {
    Stats::scoped_wait_timer wait_timer(Stats::WAIT_OP_ANY);

    common::onoff_switch sw;

    RequestIterator cur = reqs_begin, result = reqs_end;

    for ( ; cur != reqs_end; cur++)
    {
        if ((RequestPtr(*cur))->add_waiter(&sw))
        {
            // request is already done, no waiter was added to the request
            result = cur;

            if (cur != reqs_begin)
            {
                while (--cur != reqs_begin)
                    (RequestPtr(*cur))->delete_waiter(&sw);

                (RequestPtr(*cur))->delete_waiter(&sw);
            }

            (RequestPtr(*result))->check_error();

            return result;
        }
    }

    sw.wait_for_on();

    for (cur = reqs_begin; cur != reqs_end; cur++)
    {
        (RequestPtr(*cur))->delete_waiter(&sw);
        if (result == reqs_end && (RequestPtr(*cur))->poll())
            result = cur;
    }

    return result;
}

//! Suspends calling thread until \b any of requests is completed.
//! \param req_array array of \c request_ptr objects
//! \param count size of req_array
//! \return index in req_array pointing to the \b first completed request
static inline size_t wait_any(RequestPtr req_array[], size_t count) {
    return wait_any(req_array, req_array + count) - req_array;
}

//! \}

} // namespace io
} // namespace thrill

#endif // !THRILL_IO_REQUEST_OPERATIONS_HEADER

/******************************************************************************/
