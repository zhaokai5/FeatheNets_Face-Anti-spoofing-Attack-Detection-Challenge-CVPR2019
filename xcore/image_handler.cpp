/*
 * image_handler.cpp - image handler implementation
 *
 *  Copyright (c) 2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "image_handler.h"

namespace XCam {

ImageHandler::ImageHandler (const char* name)
    : _name (NULL)
{
    if (name)
        _name = strndup (name, XCAM_MAX_STR_SIZE);
}

ImageHandler::~ImageHandler()
{
    xcam_mem_clear (_name);
}

bool
ImageHandler::set_allocator (const SmartPtr<BufferPool> &allocator)
{
    XCAM_FAIL_RETURN (
        ERROR, allocator.ptr (), false,
        "softhandler(%s) set allocator(is NULL)", XCAM_STR(get_name ()));
    _allocator = allocator;
    return true;
}

bool
ImageHandler::set_first_worker (const SmartPtr<Worker> &worker)
{
    XCAM_ASSERT (worker.ptr ());
    XCAM_FAIL_RETURN (
        ERROR, !_fist_worker.ptr (), false,
        "image_handler(%s) set first worker failed. it's already exist(%s)",
        XCAM_STR(get_name()), XCAM_STR(_fist_worker->get_name ()));

#if 0
    SmartPtr<Worker> current = worker;
    while (current.ptr ()) {
        if (current.)
            current.set_callback (this);
        SmartPtr<WorkChain> chain = current->get_next ();
        current = chain->dynamic_cast_ptr<Worker> ();
        if (chain.ptr ()) {
            XCAM_ASSERT (chain.ptr ());
        }
    }
#endif

    _fist_worker = worker;
    return true;
}

XCamReturn
ImageHandler::finish ()
{
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
ImageHandler::terminate ()
{
    if (_allocator.ptr ())
        _allocator->stop ();

    SmartPtr<Worker> current = _fist_worker;
    while (current.ptr ()) {
        current->set_callback (NULL);
        SmartPtr<WorkChain> next = current->get_next ();
        current->set_next (NULL);
        current = next.dynamic_cast_ptr<Worker> ();
        if (next.ptr ()) {
            XCAM_ASSERT (next.ptr ());
        }
    }
    _fist_worker.release ();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
ImageHandler::execute_status_check (SmartPtr<ImageHandler::Parameters> &params, const XCamReturn error)
{
    if (_callback.ptr ())
        return _callback->execute_status (this, params, error);
    return error;
}

XCamReturn
ImageHandler::reserve_buffers (const VideoBufferInfo &info, uint32_t count)
{
    XCAM_FAIL_RETURN (
        ERROR, _allocator.ptr (), XCAM_RETURN_ERROR_PARAM,
        "softhandler(%s) reserve buffers failed, alloctor was not set", XCAM_STR(get_name ()));

    _allocator->set_video_info (info);

    XCAM_FAIL_RETURN (
        ERROR, _allocator->reserve (count), XCAM_RETURN_ERROR_MEM,
        "softhandler(%s) reserve buffers(%d) failed", XCAM_STR(get_name ()), count);

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<VideoBuffer>
ImageHandler::get_free_buf ()
{
    XCAM_FAIL_RETURN (
        ERROR, _allocator.ptr (), NULL,
        "softhandler(%s) get free buffer failed since allocator was not initilized", XCAM_STR(get_name ()));

    return _allocator->get_buffer (_allocator);
}

}
