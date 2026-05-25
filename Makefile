.PHONY: build test quality benchmark dashboard ml-study clean

build:
	cmake -S . -B build
	cmake --build build -j

test: build
	ctest --test-dir build --output-on-failure

quality:
	./scripts/quality/run_quality_gate.sh

benchmark: build
	./build/dispatch_lookup_structure_benchmark
	python3 performance_systems/cache_locality_experiment.py
	python3 performance_systems/kv_cache_batching_sim.py
	python3 performance_systems/threading_microbenchmark.py

dashboard:
	python3 scripts/visualization/build_performance_dashboard.py

ml-study:
	python3 ml_systems_studies/run_prefill_decode_bottleneck_sweep.py

clean:
	rm -rf build build-asan reports/benchmarks/*
