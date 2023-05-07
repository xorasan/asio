#include "thread.h"
#define XATAA 0
#define duv_buf_type(n, t) \
	duk_get_global_string(c, #n); \
	yes = duk_instanceof(c, -2, -1); \
	duk_pop(c); \
	if (yes) return t;

static void print_duk_value(duk_context *ctx, duk_idx_t i) {
	switch (duk_get_type(ctx, i)) {
		case DUK_TYPE_NONE       :
			printf("none\n"); break;
		case DUK_TYPE_UNDEFINED  :
			printf("undefined\n"); break;
		case DUK_TYPE_NULL       :
			printf("null\n"); break;
		case DUK_TYPE_BOOLEAN    :
			printf("bool %d\n", duk_get_boolean(ctx, i)); break;
		case DUK_TYPE_NUMBER     :
			printf("number %lf\n", duk_get_number(ctx, i)); break;
		case DUK_TYPE_STRING     :
			printf("string %s\n", duk_get_string(ctx, i)); break;
		case DUK_TYPE_OBJECT     :
			if (duk_is_array(ctx, i))
				printf("array object\n"); 
			else if (duk_is_buffer_data(ctx, i))
				printf("buffer object\n");
			else if (duk_is_function(ctx, i))
				printf("function object\n");
			else
				printf("object\n");
			break;
		case DUK_TYPE_BUFFER     :
			printf("buffer\n"); break;
		case DUK_TYPE_POINTER    :
			printf("pointer\n"); break;
		case DUK_TYPE_LIGHTFUNC  :
			printf("lightfunc\n"); break;
	}
}
static void print_duk_stack(duk_context *ctx) {
	duk_int_t n = duk_get_top(ctx);
	printf("duk stack size %d\n", n);
	for (int i = 0; i < n; ++i) {
		printf("%d : ", i);
		print_duk_value(ctx, i);
	}
}
duk_uint_t duv_get_buffer_flag(duk_context *c) {
	char yes;
	duv_buf_type(Int8Array,		DUK_BUFOBJ_INT8ARRAY);
	duv_buf_type(Int16Array,	DUK_BUFOBJ_INT16ARRAY);
	duv_buf_type(Int32Array,	DUK_BUFOBJ_INT32ARRAY);
	duv_buf_type(Uint8Array,	DUK_BUFOBJ_UINT8ARRAY);
	duv_buf_type(Uint16Array,	DUK_BUFOBJ_UINT16ARRAY);
	duv_buf_type(Uint32Array,	DUK_BUFOBJ_UINT32ARRAY);
	duv_buf_type(Float32Array,	DUK_BUFOBJ_FLOAT32ARRAY);
	return DUK_BUFOBJ_ARRAYBUFFER;
}
static void pretty_print(duk_context *source) {
	duk_get_global_string(source, "pretty");
	duk_dup(source, -2);
	duk_ret_t ret = duk_pcall(source, 1);
	if (ret == DUK_EXEC_SUCCESS) {}
	else {
		if (duk_is_error(source, -1)) { // Error object
			duk_safe_to_stacktrace(source, -1);
			printf("%s\n", duk_safe_to_string(source, -1));
		}
	}
	duk_pop(source);
}
void duv_xfer_obj(duk_context *source, duk_context *dest) { // IMPORTANT NOTE
	// don't parse recusrive objects
	duk_cbor_encode(source, -1, 0);
	if (duk_is_buffer_data(source, -1)) {
		duk_size_t length;
		void *buf_source, *buf_dest;
		buf_source = duk_get_buffer_data(source, -1, &length);
		if (buf_source) {
			buf_dest   = duk_push_fixed_buffer(dest, length);
			memcpy(buf_dest, buf_source, length);
			duk_push_buffer_object(dest, -1, 0, length, duv_get_buffer_flag(source));
			// if not done, leaves a buf obj on stack, making threads crash
			duk_remove(dest, -2); // remove buffer data, leave buffer object
		}
	}
	duk_cbor_decode(dest, -1, 0);
}
static void duv_fatal(void *udata, const char *msg) { // TODO cleanup -_-
	(void) udata;
	printf("fatal in thread: %s\n", (msg ? msg : "no message"));
	abort();
}
void duv_execute_thread(uv_work_t *req) {
	if (XATAA) printf("duv_execute_thread\n");
	if (!duv_thread_env.alive) return;

	duv_thread_req_t *data = req->data;
	int nargs = 1;

	duk_context *thread = data->thread;

	if (XATAA > 1) print_duk_stack(thread);

	duk_int_t rc = duk_pcall(thread, nargs);  /* [ ... func 2 3 ] -> [ 5 ] */
	if (rc == DUK_EXEC_SUCCESS) {
		duk_get_int(thread, -1);
	}
}
duv_thread_req_t *duv_cleanup_thread(duk_context *ctx, duv_thread_req_t *data) {
	if (XATAA) printf("duv_cleanup_thread\n");

	if (data->thread) {
		duk_push_global_object(data->thread);
		duk_compact(data->thread, -1); // reduces mem usage
		duk_pop(data->thread);

		duk_destroy_heap(data->thread);
		data->thread = NULL;

		duk_push_global_object(ctx);
		duk_compact(ctx, -1);
		duk_pop(ctx);
	}
	duv_unref(ctx, data->object	);
	duk_free (ctx, data			);
	return NULL;
}
void duv_yield_thread(duk_context *ctx, uv_req_t* req, int nargs) {
	if (XATAA) printf("duv_yield_thread\n");

	duv_thread_req_t *data = req->data;
	if (data->object) {
		duv_push_ref(ctx, data->object);

		duk_idx_t obj = duk_normalize_index(ctx, -1);

		duk_get_prop_string(ctx, obj, "cb");
		duk_get_prop_string(ctx, obj, "ctx");
		
		duk_remove(ctx, obj);
		
		// move args to stack top
		for (int i = 0; i < nargs; ++i) duk_pull(ctx, 0);

		duk_int_t rc = duk_pcall_method(ctx, nargs); // [fn ctx arg arg ...]
		if (rc == DUK_EXEC_SUCCESS) {}
		else {
			if (duk_is_error(ctx, -1)) duk_safe_to_stacktrace(ctx, -1);
			printf("%s\n", duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	} else if (nargs) {
		duk_pop_n(ctx, nargs);
	}
}
static void duv_thread_cb(uv_work_t *req, int result) {
	if (XATAA) printf("duv_thread_cb result %d\n", result);

	duk_context *ctx = req->loop->data;
	duv_thread_req_t *data = req->data;
	
	int nargs = 0;
	if (result < 0 || result == UV_ECANCELED) {
		duk_push_error_object(ctx, DUK_ERR_ERROR, "%s: %s", uv_err_name(result), uv_strerror(result));
		nargs = 1;
	} else {
		duk_context *thread = data->thread;
		if (XATAA > 1) print_duk_stack(thread);
		nargs = 1;
		
		if (duk_is_error(thread, -1)) { // Error object
			duk_safe_to_stacktrace(thread, -1);
			duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", duk_safe_to_string(thread, -1));
		} else {
			duv_xfer_obj(thread, ctx);
		}
		duk_pop(thread); // func return value
	}
	duv_yield_thread(ctx, (uv_req_t *) req, nargs);
	req->data = duv_cleanup_thread(ctx, data);
}
uv_work_t *duv_setup_thread(duk_context *ctx) {
	if (XATAA) printf("duv_setup_thread\n");

	/* TODO copy the main heap/context after binding and copy it to these
	 * threads, this will save cost of having to require JS libs at startup
	 * */
	duk_context *thread = duk_create_heap(NULL, NULL, NULL, NULL, duv_fatal);

	if (!duk_check_stack(thread, 2)) { // func + buffer
		/* return 'undefined' if cannot allocate space */
		printf("failed to reserve enough stack space\n");
		return NULL;
	}

	uv_work_t *req = duk_push_fixed_buffer(ctx, sizeof(*req)); // freed in cleanup
	duv_thread_req_t *data = duk_alloc(ctx, sizeof(*data));
	req->data = data;

	data->thread = thread; // reference thread

	duk_push_object(ctx); {
		duk_dup(ctx, -2); // uv_work_t buffer
		duk_put_prop_string(ctx, -2, "req");

		duk_push_this(ctx); // this object for the function
		duk_put_prop_string(ctx, -2, "ctx");

		duk_dup(ctx, 2);
		duk_put_prop_string(ctx, -2, "cb");
	}
	
	data->object = duv_ref(ctx); // eats object above

	// xfer func + data to thread's stack
	duk_dup(ctx, 1);
	duk_dump_function(ctx); // pushes a buffer
	duk_size_t out_size;
	void *ptr = duk_opt_buffer_data(ctx, -1, &out_size, NULL, 0);
	if (ptr) {
		duk_push_external_buffer(thread);
		duk_config_buffer(thread, -1, ptr, out_size);
		duk_load_function(thread);
	}
	duk_pop_n(ctx, 2); // func + buf
	
	duk_dup(ctx, 0);
	duv_xfer_obj(ctx, thread);
	duk_pop(ctx); // pop dup'd obj
	
	if (XATAA) printf("thread stack size %d\n", duk_get_top(thread));
	if (XATAA > 1) print_duk_stack(thread);

	if (duv_thread_env.bind) duv_thread_env.bind(thread);

	uv_queue_work(duv_loop(ctx), req, duv_execute_thread, duv_thread_cb);
	return req;
}
duk_ret_t duv_thread(duk_context *ctx) {
	if (duv_thread_env.alive) {
		duk_idx_t nargs = duk_get_top(ctx);
		if (nargs < 2 || !duk_is_function(ctx, 1) || !duk_is_function(ctx, 2)) return 0;

		uv_work_t *req = duv_setup_thread(ctx);
		if (!req) return 0;
		
		duv_thread_req_t *data = req->data;
		duv_push_ref(ctx, data->object);
		return 1;
	}
	return 0;
}
