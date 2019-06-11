#!/usr/bin/env python3
from waflib import Configure, Options
Configure.autoconfig = True

def options(ctx):
    ctx.load('compiler_cxx')
    group = ctx.get_option_group('configure options')
    group.add_option('--google_benchmark_libpath', default='', type='string')
    group.add_option('--google_benchmark_incpath', default='', type='string')

@Configure.conf
def check_google_benchmark(ctx, with_pthread):
    libs = ['benchmark']
    if with_pthread:
        libs.append('pthread')
    ctx.check_cxx(lib=libs,
                  includes=Options.options.google_benchmark_incpath,
                  libpath=Options.options.google_benchmark_libpath,
                  fragment="#include <benchmark/benchmark.h>\nint main(){}",
                  uselib_store='GOOGLE_BENCHMARK', mandatory=with_pthread)

def configure(ctx):
    ctx.load('compiler_cxx')
    ctx.check_cxx(lib='blas', uselib_store='BLAS')
    if not ctx.check_google_benchmark(with_pthread=False):
        ctx.check_google_benchmark(with_pthread=True)

def build(ctx):
    ctx.program(target='bench_dot_product',
                source=[ctx.path.find_node('bench_dot_product.cpp')],
                use=['BLAS', 'GOOGLE_BENCHMARK'],
                cxxflags=['-Wall', '-Wextra', '-Werror', '-O3', '-ffast-math'])
