BENCHMARKS=peakflops_avx_fma peakflops_sp_avx_fma copy_avx load_avx store_avx
BENCHMARK_OBJS_LIKWID=$(addsuffix .o,$(addprefix GCC/, $(BENCHMARKS)))
BENCHMARK_OBJS=$(addprefix likwid/bench/, $(BENCHMARK_OBJS_LIKWID))

benchmarks: 
	cd likwid/bench && make $(BENCHMARK_OBJS_LIKWID) -j$(shell nproc)

clean:
	@rm -f $(BENCHMARK_OBJS)
