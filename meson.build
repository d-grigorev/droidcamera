project('droidcamera', 'c',
  version : '0.1',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized' ])

droidcamera_version = meson.project_version()
droidcamera_name = meson.project_name()

cc = meson.get_compiler('c')

root_dir = include_directories('.')

libdroidcamera_plugin_install_dir = '/'.join([ get_option('libdir'), meson.project_name(), meson.project_version(), 'plugin'])
add_project_arguments('-DDROID_CAMERA_PLUGIN_DIR="' +
                      '/'.join([get_option('prefix'), libdroidcamera_plugin_install_dir]) +
                      '"', language: 'c')

if get_option('build-plugin') == true
        droidmedia_h = include_directories('/usr/include/droidmedia')
        hybris_dep = cc.find_library('hybris-common', required: true)

        droidcamera_plugin_source = [
          'droidcamera_plugin.c',
          '/usr/share/droidmedia/hybris.c',
        ]

        droidcamera_plugin = shared_module('droidcamera-plugin',
                               droidcamera_plugin_source,
                               include_directories: [ droidmedia_h ],
                               install: true,
                               dependencies: hybris_dep,
                               install_dir: libdroidcamera_plugin_install_dir )
endif

if get_option('build-library') == true or get_option('build-tests') == true
        droidcamera_source = [
          'utils.c',
          'droidcamera.c',
        ]

        # Use libtool-version numbers
        libdroidcamera_lt_c=0
        libdroidcamera_lt_r=0
        libdroidcamera_lt_a=0

        libdroidcamera_so_version = '@0@.@1@.@2@'.format((libdroidcamera_lt_c - libdroidcamera_lt_a),
                                                         libdroidcamera_lt_a,
                                                         libdroidcamera_lt_r)

        libdroidcamera_so = shared_library('droidcamera',
                               droidcamera_source,
                               install: true,
                               dependencies: cc.find_library('dl', required : false),
                               version: libdroidcamera_so_version )

        install_headers('droidcamera.h', subdir : meson.project_name())
endif

if get_option('build-tests') == true
        unittest_source = [
          'utils.c',
          'unittest.c'
        ]

        unittest = executable('unittest',
                               unittest_source,
                               install: false)

        testapp_source = [
          'testapp.c'
        ]

        testapp = executable('testapp',
                               testapp_source,
                               install: false,
                               link_with: libdroidcamera_so )
endif
