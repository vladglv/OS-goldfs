project('os-goldfs', 'c', default_options : ['c_std=c11'], version : '0.2')

compiler = meson.get_compiler('c').get_id()

cc_args = []
cc_args_exe = []
la = []

if compiler == 'clang' or compiler == 'gcc'
  cc_args += [
    '-Wmain',
    '-W',
    '-Wall',
    '-Wextra',
    '-D_XOPEN_SOURCE=700',
    '-Wformat',
    '-Wformat-security',
    '-Werror=format-security',
    '-Wpedantic',
    '-pedantic',
    '-pedantic-errors',
    '-fstack-protector-strong',
    '-fsanitize=address,undefined,leak',
    ]

  if compiler == 'clang'
    cc_args += [
      '-Wthread-safety',
      '-Weverything',
      ]
  endif

  cc_args_exe = cc_args + ['-fPIC']
  la += ['-lpthread', '-ldl', '-lasan', '-lubsan']
  #la += ['-ldisk_emu']
  #la += ['-L' + meson.build_root() + '/../disk_emu/target/x86_64-unknown-linux-musl/release/']
  #la += ['-L' + meson.build_root() + '/../disk_emu/target/release/']
else
  warning('c_args is only configured for clang and gcc')
  warning('The build may work but is untested')
endif

message('buildtype=' + get_option('buildtype'))

src_disk_emu = ['src/disk_emu.c']
src_gold_fs = ['src/sfs_api.c']
src_test_common = ['src/tests.c']

src_test_1 = ['src/sfs_test1.c']
src_test_2 = ['src/sfs_test2.c']

libdisk_emu = static_library('disk-emu', src_disk_emu, c_args : cc_args)
libgold_fs = static_library('gold-fs', src_gold_fs, c_args : cc_args)
libtest_common = static_library('test-common', src_test_common, c_args : cc_args)

deps = [libdisk_emu, libgold_fs, libtest_common]

executable('sfs_t1', src_test_1, c_args : cc_args_exe, link_with : deps, link_args : la)
executable('sfs_t2', src_test_2, c_args : cc_args_exe, link_with : deps, link_args : la)

