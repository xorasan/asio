// prints a dir tree upto <depth>
"use strict";

var p = print, dirs = {}, depth = 2; // you can change this var
var buf_to_str = function (buf) {
	return String.fromCharCode.apply(null, new Uint16Array(buf));
};
var data_unit = function (v) { return parseFloat(v/1024).toFixed(2)+'k'; };
var tree = function (dirs, dep) {
	dep = dep || 0;
	for (var i in dirs) {
		var o = dirs[i];
		var str = dep ? '   '.repeat(dep) : '', size = '', type = '';
		if (o.children && o.checked) {
			size = ('number', ' <'+Object.keys(o.children).length+'>');
		} else {
			if (o.size == -1)
			size = ' ...';
			else
			size = ('number', ' ['+data_unit(o.size)+']');
		}
		if (o.type == 'directory') type = 'dir';
		
		str += ('string', o.name)+size+' '+('property', type);
		print(str);
		if (o.children && dep < depth) {
			tree(o.children, dep+1);
		}
	}
};

var read_callback = function (fd) {
	p('open', fd);
	uv.fs_fstat(fd, function (stats) {
		p('fstat', stats.size);
		uv.fs_read(fd, stats.size, -1, function (data) {
			p('read', buf_to_str(data).slice(0, 40));
			uv.fs_close(fd);
		});
	});
};
var get_size = function (path, store) {
	uv.fs_open(path, "", 0, function (fd) {
		if (fd)
		uv.fs_fstat(fd, function (stats) {
			store[path].size = stats.size;
			
			uv.fs_close(fd);
		});
	});
};
var scandir = function (path, parent, dep) {
	dep = dep || 0;
	var store = parent || dirs;
	if (dep < depth) {
		uv.fs_scandir(path, function (userdata) {
			if (path.endsWith('/')) path = path.slice(0, -1);
			var o;
			if (userdata)
			while (o = uv.fs_scandir_next(userdata)) {
				var full_path = path+'/'+o.name;
				store[full_path] = {
					name: o.name,
					type: o.type,
					size: -1,
					checked: 0,
				};
				var child = store[full_path];
				if (o.type == 'file') get_size(full_path, child.children || store);
				if (o.type == 'directory') {
					child.children = child.children || {};
					child.checked = scandir(full_path, child.children || store, dep+1);
				}
			}
		});
		return 1;
	} else {
		return 0;
	}
};

scandir('build');

uv.run();
tree(dirs);
//uv.write(utils.stdout, "exiting\n");
