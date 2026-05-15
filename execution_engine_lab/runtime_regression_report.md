# Runtime Regression Gate Report

> Scope: CI-style regression gate for toy execution-engine labs. This is not production VM, JIT, V8, or WebAssembly validation infrastructure.

| Check | Status | Detail |
|---|---|---|
| semantic_equivalence_total_cases | PASS | observed=4 |
| semantic_equivalence_expected_mismatch_count | PASS | observed_mismatches=1 |
| semantic_equivalence_expected_mismatch_case | PASS | mismatch_cases=['intentional_mismatch_detected'] |
| runtime_equivalence_no_semantic_mismatches | PASS | semantic_mismatches=0 |
| runtime_equivalence_deopt_fallback_matches | PASS | deopt_fallback_matches=6 |
| runtime_equivalence_invalid_rejections_preserved | PASS | invalid_trace_rejections_preserved=1 |
| wasm_malformed_total_traces | PASS | total_traces=7 |
| wasm_malformed_no_false_accepts | PASS | false_accepts=0 |
| wasm_malformed_regression_status | PASS | regression_status=pass |
| tiered_execution_guard_failure_reasons | PASS | observed=['branch-direction-changed', 'operand-type-changed'] |
| inline_cache_terminal_state_load_x | PASS | observed=megamorphic_generic_fallback expected=megamorphic_generic_fallback |
| inline_cache_terminal_state_load_y | PASS | observed=polymorphic expected=polymorphic |

## Summary

- Total runtime checks: 12
- Checks passed: 12
- Checks failed: 0
- Semantic drift detected: false
- False accepts: 0
- Regression gate status: `PASS`

## Safe interpretation

This gate compares current runtime experiment outputs against checked-in expectations and flags semantic drift, malformed-trace false accepts, deoptimization classification drift, and inline-cache state regressions.
