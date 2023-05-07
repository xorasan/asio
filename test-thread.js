// .thread( data, worker, callback )
"use strict";
var pretty = require('./modules/utils.js').prettyPrint;
var thread_func = function (data) {
	data.proc = 'yes';
	return data;
};
var thread_callback = function (data) {
	pretty(obj, data);
};
var obj = {
	uid : 1,
	unde: undefined,
	null: null,
	bool: true,
	numb: 2000.0002,
	stri: "this is a string",
	obje: { prop: { value: 'nest' } },
	arra: [0, 1, 2, 3, 4],
	log: print,
	ArrayBuffer: new ArrayBuffer([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]),
	Uint8Array: new Uint8Array([10, 20, 30, 40, 50, 60, 70, 80]),
	Int8Array: new Int8Array([10, 20, 30, 40, 50, 60, 70, 80]),
	Int16Array: new Int16Array([10, 20, 30, 40, 50, 60, 70, 80]),
	Uint16Array: new Uint16Array([10, 20, 30, 40, 50, 60, 70, 80]),
	Float32Array: new Float32Array([1.1, 1.2, 1.3, 1.4]),
};
uv.thread(obj.ArrayBuffer, thread_func, thread_callback);

//for (var i = 0; i < 1; ++i) {
//	uv.thread({
//		uid: i,
//	}, thread_func, thread_callback);
//}

uv.run();
