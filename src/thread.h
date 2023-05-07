#ifndef DUV_THREAD_H
#define DUV_THREAD_H

#include "duv.h"

duk_ret_t duv_thread(duk_context *ctx);
duv_thread_req_t *duv_cleanup_thread(duk_context *ctx, duv_thread_req_t *data);

#endif
