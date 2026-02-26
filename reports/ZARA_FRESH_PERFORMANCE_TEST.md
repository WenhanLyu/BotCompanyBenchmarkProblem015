# Fresh Performance Test Results - Post Hash Change (Commit 7f09162)

Test Date: 2026-02-25
Code Version: commit 7f09162 [Elena] Replace std::hash with deterministic FNV-1a hash
Test File: stress_100k.txt (100,100 operations)
Platform: Darwin 24.6.0

## Run 1
Real time: **24.32 seconds**
Maximum resident set size: 1,572,864 bytes (1.50 MiB)
Peak memory footprint: 1,393,536 bytes (1.33 MiB)
Status: **FAIL** (time: 24.32s > 16s)

## Run 2
Real time: **24.17 seconds**
Maximum resident set size: 1,638,400 bytes (1.56 MiB)
Peak memory footprint: 1,459,072 bytes (1.39 MiB)
Status: **FAIL** (time: 24.17s > 16s)

## Run 3
Real time: **24.22 seconds**
Maximum resident set size: 1,572,864 bytes (1.50 MiB)
Peak memory footprint: 1,360,640 bytes (1.30 MiB)
Status: **FAIL** (time: 24.22s > 16s)

## Overall Verdict

**Time requirement (≤16s): FAIL** ❌
- Run 1: 24.32s (FAIL)
- Run 2: 24.17s (FAIL)
- Run 3: 24.22s (FAIL)
- Average: 24.24s (51.5% over limit)

**Memory requirement (≤6 MiB): PASS** ✓
- All runs: 1.5-1.6 MiB maximum resident set size
- All runs: 1.3-1.4 MiB peak memory footprint
- Well within 6 MiB limit

**Final: FAIL** ❌

## Critical Finding

The current code (after FNV-1a hash replacement) **FAILS the M5 time requirement by ~50%**. All three independent runs with clean data files consistently show ~24 seconds execution time, far exceeding the 16-second requirement.

The previous reports claiming <16s performance were likely based on:
1. Tests run BEFORE the hash replacement commit
2. Cached data files that reduced I/O operations
3. Different test conditions

## Recommendation

**IMMEDIATE ACTION REQUIRED**: The hash replacement introduced a significant performance regression. The code needs optimization or the hash implementation needs review to meet the M5 milestone requirements.
