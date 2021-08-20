project(
	'unudhcpd',
	'c',
	version : '0.1',
	default_options : ['warning_level=3'],
	meson_version : '>=0.53.0',
)
src = [
	'main.c',
	'server.c',
]

exe = executable(
	'unudhcpd',
	src,
	install : true
)
