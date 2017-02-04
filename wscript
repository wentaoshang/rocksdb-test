# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.0.1'
APPNAME='rocksdb_test'

def options(opt):
	opt.load(['compiler_cxx', 'cxx', 'gnu_dirs'])

ROCKSDB_CODE = '''
#include <rocksdb/db.h>
int main() {
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status status =
    rocksdb::DB::Open(options, "/tmp/testdb", &db);
  if (status.ok()) return 0;
  else return 1;
}
'''

CXXFLAGS_COMMON = ['-O2', '-std=c++11', '-stdlib=libc++']

def configure(conf):
	conf.load(['compiler_cxx', 'cxx', 'gnu_dirs'])
        conf.check_cxx(cxxflags=CXXFLAGS_COMMON, mandatory=True)
	conf.check_cxx(lib='rocksdb', fragment=ROCKSDB_CODE, execute=True, uselib_store='ROCKSDB', mandatory=True,
                       includes='/usr/local/include', libpath='/usr/local/lib', cxxflags=CXXFLAGS_COMMON)
        conf.check_cfg(package='libndn-cxx', uselib_store='NDNCXX', args=['--cflags', '--libs'], mandatory=True)

def build(bld):
	bld.program(features='cxx cxxprogram',
                    source='main.cpp',
                    target='test',
                    use='ROCKSDB NDNCXX',
                    cxxflags=CXXFLAGS_COMMON)
