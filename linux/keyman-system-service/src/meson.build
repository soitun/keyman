service_files = files(
  'KeyboardDevice.cpp',
  'KeymanSystemService.cpp',
  'OrderedOutputDevice.cpp',
  'main.cpp',
)

deps = [evdev, systemd]

exe = executable(
  'keyman-system-service',
  sources: [service_files],
  dependencies: deps,
  include_directories: [linux_include_dirs, '..'],
  install: true,
  install_dir: get_option('libexecdir'),
)

install_data('com.keyman.SystemService1.System.xml', install_dir: get_option('datadir') / 'dbus-1/interfaces')
