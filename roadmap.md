# Project Roadmap - File Storage (Problem 015)

## Project Goal
Implement a high-quality file-based key-value database for ACMOJ Problem 2545 that handles insert/delete/find operations with strict memory constraints (5-6 MiB).

## Current Status
- **Phase**: ACTIVE - M5.1 in progress
- **Date**: 2026-02-25 (Cycle 26)
- **Submission Budget**: 5 attempts remaining (2 used: 750119, 750120)
- **State**: M5 revealed critical blockers upon re-evaluation - fixing before OJ submission
- **Current Implementation**: Polynomial rolling hash with streaming I/O
- **Performance**: Memory 1.5 MiB (excellent), Time 15-19s depending on optimization (OVER LIMIT without fixes)
- **Critical Issues Found**: CMakeLists.txt missing -O2, file count violation, O(n^2) insert performance

## Architecture Decision

**Chosen Approach**: Hash-based bucketing with 20 files (Lucas's design)
**Rationale**: Simpler implementation, adequate performance (O(n/20)), lower risk for limited submission budget

## Milestones

### M1: Working Skeleton with Insert/Find
**Status**: ✅ COMPLETE
**Actual Cycles**: 2
**Description**: Build minimal working system that passes sample test
- CMakeLists.txt and .gitignore for build system
- main.cpp with command parser (insert/find only)
- BucketManager class with hash bucketing
- Binary file I/O for 20 bucket files
- Compiles to `code` executable
- Passes sample test from README
**Completion Notes**:
- Implemented by Elena and Dmitri
- Sample tests pass
- No memory leaks

### M2: Complete Implementation with Delete
**Status**: ✅ COMPLETE
**Actual Cycles**: 1
**Description**: Add delete operation and handle all edge cases
- Implemented physical deletion (rewrite bucket)
- Handles "delete non-existent" gracefully
- All edge cases pass
**Completion Notes**:
- Implemented by Elena
- All basic functionality working
- **BUT: Fails at maximum scale (see M3)**

### M3: Memory Optimization for Maximum Scale
**Status**: ✅ COMPLETE
**Actual Cycles**: 3
**Description**: Optimize implementation to handle 100K operations within resource limits
**Achievement**:
- Final implementation: 1.44 MiB for 100K operations (24% of 6 MiB limit)
- Final implementation: 14.4s CPU time for 100K operations (90% of 16s limit)
- Solution: Implemented streaming-based processing for all operations
**Implementation**:
- Insert: Single file open with read+write mode, stream to check duplicates, append
- Find: Stream through file entry-by-entry, collect matches, sort results
- Delete: Stream from input to temp file, skip deleted entry, atomic rename
**Verification**:
- Memory: 1.4 MiB peak (4.5 MiB margin) - ✅ EXCELLENT
- CPU Time: 14.3-14.5s (1.5s margin) - ✅ ACCEPTABLE
- Correctness: 38/38 test cases pass - ✅ VERIFIED
- Architecture: Streaming implementation superior to original design - ✅ APPROVED
**Completion Notes**:
- Independent evaluation by Lucas, Sophia, Maya - all recommend approval
- Ready for OJ submission

### M4: Final Testing and Submission Prep
**Status**: ✅ COMPLETE → ❌ OJ FAILED
**Actual Cycles**: 0 (covered in M3 verification)
**Description**: Final validation before submission
**Completed Verification**:
- ✅ 100K operations pass all constraints (memory + time)
- ✅ Sample test produces exact expected output
- ✅ Binary format verified at byte level
- ✅ Persistence across program runs verified (locally)
- ✅ Edge cases tested (INT_MAX, 64-byte keys, duplicates, deletions)
- ✅ No memory leaks (RAII compliant, no manual allocation)
- ✅ Compilation verified (CMakeLists.txt produces `code` executable)
- ✅ .gitignore configured correctly for OJ submission
**OJ Submission Results**:
- Submission 750119: FAILED (Score: N/A)
- Submission 750120: FAILED (Score: N/A)
**Root Cause**:
- std::hash<std::string> is non-portable across platforms/compilers
- OJ platform produces different hash values than local environment
- Persistence tests fail (data written to different buckets than expected)

### M5: Fix Hash Portability Issue
**Status**: ✅ COMPLETE → ❌ CRITICAL ISSUES FOUND IN RE-EVALUATION
**Actual Cycles**: 3 (Ares implementation) + evaluation (Athena)
**Description**: Replace non-portable std::hash with deterministic hash function
**Implementation**:
- Elena replaced std::hash with polynomial rolling hash (prime 31)
- Uses uint32_t for fixed-width arithmetic (portable across platforms)
- Correctly casts char to unsigned char (handles platform signedness)
**Initial Verification (Cycle 25)**:
- Lucas: ✅ 80% confidence
- Maya: ✅ 85% confidence (9.2/10 code quality)
- Sophia: ✅ Approved
- Athena: Marked PROJECT COMPLETE
**Re-evaluation (Cycle 26 - BLIND)**:
- Code Quality Auditor: ❌ NO-GO
- Performance Evaluator: ❌ NO-GO
**CRITICAL ISSUES FOUND**:
1. **CMakeLists.txt missing -O2**: OJ builds without optimization → 18.6s (FAILS 16s limit)
2. **File count violation**: Delete creates `.tmp` files → 21 files (exceeds 20-file limit)
3. **O(n^2) insert performance**: Full bucket scan on every insert → 15-19s depending on test case
**Root Cause of Missed Issues**:
- Cycle 25 evaluators didn't test actual OJ build process (without CMAKE_BUILD_TYPE)
- Didn't measure worst-case performance scenarios
- Underestimated time margin risk

### M5.1: Fix OJ Submission Blockers
**Status**: ❌ FAILED - Ares did zero work, all 3 fixes unimplemented
**Cycles Used**: 3/3 (wasted)
**Root Cause**: Milestone too broad with 3 distinct fixes ranging from trivial to complex
**Decision**: Break into 3 focused sub-milestones (M5.1.1, M5.1.2, M5.1.3)
**Required Fixes**:
1. **CMakeLists.txt optimization** (CRITICAL, EASY)
   - Move `-O2` from CMAKE_CXX_FLAGS_RELEASE to base CMAKE_CXX_FLAGS
   - Ensure OJ build process includes optimization
2. **File count violation** (CRITICAL, DESIGN CHANGE)
   - Eliminate `.tmp` file creation in delete operation
   - Options: in-place rewrite, flag-based soft delete, or alternative strategy
   - Must stay within 20-file limit at all times
3. **Insert performance optimization** (CRITICAL, ARCHITECTURAL)
   - Current: O(n^2) due to full bucket scan on every insert
   - Options:
     a. ~~Remove duplicate check entirely~~ (CANNOT - spec requires duplicate prevention)
     b. Optimize duplicate check (sorted bucket + binary search)
     c. Use in-memory bloom filter or hash set for current session
   - Target: 100K operations must complete in <14s (leave 2s margin below 16s limit)

### M5.1.1: Fix CMakeLists.txt Optimization Flag
**Status**: ✅ COMPLETE
**Cycles Used**: 1
**Completion**: Ares moved -O2 to base CMAKE_CXX_FLAGS, verified by build test
**Verification**: Sophia confirmed -O2 active in compiler commands

### M5.1.2: Eliminate Temp Files in Delete Operation
**Status**: ✅ COMPLETE
**Cycles Used**: 2
**Completion**: Ares modified delete_entry to use in-place rewrite (read → close → truncate write)
**Verification**: Marcus confirmed zero temp files created in all edge cases

### M5.1.3: Fix O(n²) Insert Performance with In-Memory Index
**Status**: ❌ FAILED - Wrong architectural direction taken
**Cycles Used**: 4/4 (all wasted)
**Root Cause**: Ares's team implemented bounded cache instead of unbounded cache
**What Happened**:
- Elena implemented single-file architecture with bounded LRU cache (commit 6d93bcb)
- MAX_INDEX_ENTRIES = 15,000 with LRU eviction
- After cache fills, every insert scans entire file → O(n²) behavior WORSE than before
**Catastrophic Results** (Felix & Sophia evaluations):
- Random test: 77.17s (4.8x over 16s limit) ❌
- Insert-heavy: 202.42s (12.7x over limit) ❌
- Collision: 77.48s (4.8x over limit) ❌
- Memory: 41-71 MB (7-12x over 6 MiB limit) ❌
- File count: 1 ✅ (only success)
**Why It Failed**:
1. **Bounded cache without bloom filter** = worst of both worlds
   - After 15K entries, degrades to O(n²) file scans
   - LRU structures add 3x memory overhead
   - Delete loads entire file into memory
2. **Wrong optimization target**: Optimized file count while violating ALL other constraints
3. **Spec misinterpretation**: Felix thought in-memory cache violated spec, but Maya proved it's allowed

**Key Findings from Post-Mortem**:
- Lucas (Issue #50): "Bounded cache fundamentally flawed without bloom filter"
- Sophia (Issue #49): "85% OJ TLE probability, DO NOT SUBMIT"
- Maya (Issue #51): "Unbounded cache IS allowed by spec - 'current operation' = program execution"
**Lesson Learned**:
- Unbounded cache needed BUT 100K entries × 70 bytes = 7 MB exceeds 6 MiB limit
- Must use bounded cache (15K entries ≈ 1 MB) + bloom filter (~125 KB) to prevent file scans
- 20-file architecture is required (not 1 file, not 5000 files)

### M5.1.4: Add Bloom Filter to Prevent O(n²) File Scans
**Status**: ❌ FAILED - Single-file architecture fundamentally too slow
**Cycles Used**: 3/3
**Description**: Add Bloom filter to bounded cache implementation to eliminate O(n²) degradation after cache fills
**Why Current Implementation Fails** (Commit 6d93bcb Analysis):
- Single file with bounded cache (15K entries max) + LRU eviction
- After 15K entries, EVERY insert triggers full file scan via `check_file_for_duplicate()`
- Result: O(n²) complexity → 77-202 seconds (4.8-12.7x over 16s limit)
- Memory: 41-71 MB due to LRU overhead + delete loading entire file
**Root Cause**: Bounded cache without probabilistic filter
- Can't cache all entries (7 MB exceeds 6 MiB limit)
- Can't scan file on every miss (O(n²) too slow)
- **Solution**: Bloom filter for negative lookups
**What Is Bloom Filter**:
- Probabilistic data structure for set membership testing
- Space: ~100-150 KB for 100K entries with 1% false positive rate
- Operations: O(1) add, O(1) query
- Property: Never has false negatives (if filter says "not present", it's guaranteed absent)
- False positive rate: ~1% (tunable)
**Implementation Plan**:
1. **Add bloom filter** (std::bitset or custom bit array):
   ```cpp
   // In bucket_manager.h
   std::bitset<800000> bloom_filter_;  // ~100 KB for 100K entries

   // 3 hash functions for k=3 (optimal for 1% FP rate)
   size_t hash1(const std::string& index, int value);
   size_t hash2(const std::string& index, int value);
   size_t hash3(const std::string& index, int value);
   ```

2. **Modify insert_entry** (bucket_manager.cpp:120-162):
   ```cpp
   // Check bounded index first
   if (index_.find(key) != index_.end()) {
       return;  // Duplicate in cache
   }

   // Check bloom filter
   if (!bloom_filter_contains(index, value)) {
       // DEFINITELY not a duplicate - safe to insert
       append_to_file(index, value);
       add_to_bloom_filter(index, value);
       add_to_index_with_eviction(key);
   } else {
       // MIGHT be duplicate - check file
       if (check_file_for_duplicate(index, value)) {
           return;  // Duplicate found
       }
       // Not a duplicate, insert
       append_to_file(index, value);
       add_to_bloom_filter(index, value);
       add_to_index_with_eviction(key);
   }
   ```

3. **Fix delete memory bloat** (bucket_manager.cpp:252-301):
   - Current: Loads entire file into vector (causes 41-71 MB spikes)
   - Solution: Use in-place rewrite (read → close → truncate write) - same as M5.1.2 fix
   - Or use streaming rewrite with temporary buffer (not temp file)

4. **Reduce LRU overhead**:
   - Current: 3 data structures (index_, lru_list_, lru_pos_) = 3x memory overhead
   - Alternative: Use single std::unordered_map + timestamp for simpler LRU
   - Or accept current overhead if bloom filter solves performance

**Expected Impact**:
- Bloom filter false positive rate: ~1%
- 99% of inserts after cache full: O(1) (bloom filter says "not present")
- 1% of inserts: O(n) file scan (bloom filter false positive)
- Overall: 99x improvement over current O(n²) behavior
- **Estimated performance**:
  - Random test: 77s → **5-8s** ✅
  - Insert-heavy: 202s → **8-12s** ✅
  - Collision: 77s → **6-10s** ✅
- **Memory**: Bloom filter ~100 KB + bounded cache ~1.5 MB + overhead ~1 MB = **~3 MB** ✅

**Success Criteria**:
- ✅ All three performance tests: <16s
- ✅ Memory: <6 MiB (target <4 MiB for safety margin)
- ✅ File count: ≤20 files (current: 1 file)
- ✅ Sample test: passes with correct output
- ✅ No regression on correctness (bloom filter never causes false negatives)

**Risks**:
- **LOW**: Bloom filter is well-understood, easy to implement
- Bit manipulation is straightforward with std::bitset
- False positive rate tunable via filter size and hash count
- Main risk: Bloom filter implementation bugs → mitigate with thorough testing

**Alternative If This Fails**:
- Switch to 20-bucket architecture (commit c5147e3) with adaptive cache eviction
- Request clarification on 20-file limit interpretation

**ACTUAL OUTCOME (Cycle 27-29)**:
- Elena implemented bloom filter (commit 8ed1074) ✅
- Maya fixed delete memory bloat (commit 37c1bee) ✅
- Felix tested commit 37c1bee: **ALL TESTS FAILED** ❌
  - Random: 78.51s (4.9x over 16s limit)
  - Collision: 223.42s (14.0x over limit)
  - Insert-heavy: 379.16s (23.7x over limit)
  - Memory: 13-131 MiB (2-22x over 6 MiB limit)
- **Root Cause**: Single-file architecture is fundamentally too slow
  - Bloom filter works but doesn't solve the file I/O bottleneck
  - Every operation touches the same file → no parallelism
  - Find operations still scan entire file (no index)
- **Key Finding** (Maya's blind audit): Find operations have O(n) complexity, causing catastrophic performance

### M5.2: Switch to Multi-Bucket Architecture
**Status**: 🎯 READY TO START
**Cycles Allocated**: 4
**Description**: Replace single-file architecture with 10-bucket system using unbounded in-memory cache of compact entries

**Why Single-File Failed**:
The single-file approach (M5.1.3, M5.1.4) was fundamentally flawed:
- All operations contend on one file → excessive I/O overhead
- Find operations require full file scan → O(n) per find
- Delete requires file rewrite → O(n) per delete
- No way to parallelize or distribute load
- Roadmap explicitly states: "20-file architecture is required (not 1 file, not 5000 files)"

**Why Multi-Bucket Works**:
- Distribute 100K entries across 10 buckets → 10K entries per bucket
- Each operation only touches 1 bucket (1/10th of data)
- Unbounded cache keeps all entries in memory → O(1) operations
- Memory: Use compact entries (hash+value) instead of full strings

**Architecture Specification**:

1. **Bucket Configuration**:
   ```cpp
   NUM_BUCKETS = 10  // Files: 10/20 (50% margin)
   ```

2. **Compact Entry Structure**:
   ```cpp
   struct CompactEntry {
       uint32_t key_hash;    // hash(index) for fast lookup
       int32_t value;        // the value
       int64_t file_offset;  // for full string verification
   };
   // Memory: 16 bytes per entry
   ```

3. **Per-Bucket Cache**:
   ```cpp
   // One map per bucket, maps key_hash -> list of (value, offset) pairs
   std::array<std::unordered_map<uint32_t, std::vector<std::pair<int32_t, int64_t>>>, 10> bucket_cache_;
   ```

4. **Memory Calculation**:
   - 100K entries × 16 bytes = 1,600,000 bytes = 1.53 MiB (base)
   - Hash table overhead (~50%): +800 KB
   - Vector overhead (~20%): +320 KB
   - **Total: ~2.6 MiB** (57% margin, safe!)

5. **Expected Performance**:
   - Insert: O(1) hash lookup + O(log k) for k values per index
   - Find: O(1) hash lookup + O(k log k) sort
   - Delete: O(1) hash lookup + file rewrite of 1 bucket
   - **Estimated time**: 8-12s for 100K operations (25-50% margin)

**Implementation Requirements**:
1. Remove all bloom filter code (~150 lines)
2. Remove LRU eviction code (~80 lines)
3. Change NUM_BUCKETS from 1 to 10
4. Implement CompactEntry structure
5. Load each bucket file into cache on first access (lazy-load)
6. On cache hit, verify full string by reading file at offset
7. Handle hash collisions (rare, ~0.00002% probability)

**Success Criteria**:
- ✅ Time: <16s for all three 100K tests (target: 8-12s)
- ✅ Memory: <6 MiB (target: <4 MiB for safety)
- ✅ Files: ≤20 (actual: 10 files)
- ✅ Correctness: Sample test passes, no regressions

**Risks**:
- LOW: Compact entry approach is well-proven
- Hash collision handling is straightforward
- 57% memory margin provides safety buffer

**Fallback If This Fails**:
- Increase NUM_BUCKETS to 15-19 for even smaller buckets
- Use full strings if compact entries have issues (will use ~4-5 MiB)

## Key Constraints
- Memory limit: 5-6 MiB per test case
- Time limit: 500-16000 ms per test case
- Max 100,000 commands
- Must use file storage (not in-memory)
- Must compile to executable named `code`
- C/C++ only

## Technical Challenges
1. Efficient file-based storage with minimal memory usage
2. Fast find operations with sorted output
3. Handling persistence across test cases
4. Staying within file count limit (20 files)

## Lessons Learned

### Cycle 1 (Planning - Athena)
- **Decision**: Chose hash-based bucketing over B+ tree for simplicity
- **Research**: Both Lucas and Sophia provided excellent analysis
- **Strategy**: Prioritize working code over optimal complexity for limited submission budget

### Cycle 2-3 (Implementation & Verification - Ares/Apollo/Athena)
- **Achievement**: Basic functionality complete (all operations working)
- **Critical Finding**: Implementation fails at maximum scale
  - 100K operations: 6.53 MiB (exceeds 6 MiB limit) + 41.19s (exceeds 16s)
  - Root cause: Loading entire buckets into memory
- **Research**: Sophia's performance testing identified the scale issue early
- **Key Insight**: Functional correctness ≠ production readiness. Must test at maximum scale.
- **Action**: Optimize for streaming/chunked processing instead of loading full buckets

### Cycle 4-5 (Optimization & Final Verification - Ares/Athena)
- **Achievement**: Streaming implementation complete
  - Memory: 1.44 MiB (76% improvement, 24% of limit)
  - Time: 14.4s CPU time (65% improvement, 90% of limit)
- **Implementation**: Elena executed streaming optimization for all three operations
  - Insert: Single file open, stream for duplicate check, append
  - Find: Stream through file, collect matches, sort
  - Delete: Stream to temp file, skip deleted entry, atomic rename
- **Independent Evaluation**: Athena commissioned blind evaluations by Lucas, Sophia, Maya
  - Lucas (Architecture): ✅ Approved - streaming implementation superior to original design
  - Sophia (Performance): ⚠️ Conditional pass - memory excellent, time tight (90% utilization)
  - Maya (Correctness): ✅ Approved - 38/38 tests pass, binary format verified
- **Key Decision**: Accept 90% time utilization as acceptable for OJ submission
  - Most OJs measure CPU time (not wall clock)
  - 7 submission attempts available for iteration if needed
  - Further optimization has diminishing returns and risks introducing bugs
- **Final Verdict**: Code ready for external OJ evaluation

### Cycle 6 (OJ Submission & Failure Analysis - External/Athena)
- **OJ Submissions**: 2 attempts failed (750119, 750120)
- **Investigation**: Athena commissioned blind investigations by Lucas, Sophia, Maya
  - Sophia (Issue #20): 🚨 CRITICAL - std::hash<std::string> non-portable across platforms
  - Maya (Issue #21): ⚠️ MEDIUM - File operation error handling in delete
  - Lucas (Issue #22): ✅ Build system requirements verified
- **Root Cause**: std::hash produces different values on OJ platform vs local
  - Impact: Persistence tests fail (data in wrong buckets)
  - Evidence: 2 failed submissions + Sophia's research on hash portability
- **Key Lesson**: Platform-specific code (std::hash) breaks cross-platform testing
  - Local testing passes != OJ passes
  - Must use portable, deterministic algorithms for competitive programming
  - Standard library implementations can vary across platforms
- **Next Action**: Replace std::hash with portable hash (M5)

### Cycle 7-9 (M5 Implementation - Ares/Athena)
- **Achievement**: Hash portability fixed
  - Cycle 7: Elena replaced std::hash with FNV-1a (too slow at 24s)
  - Cycle 8: Elena switched to polynomial rolling hash (14.4s)
  - Cycle 9: Felix verified + Athena's team evaluated (Lucas, Maya, Sophia)
- **Cycle 25 Evaluation**: Three evaluators recommended OJ submission (80-85% confidence)
- **Decision**: Athena marked PROJECT COMPLETE
- **Outcome**: Orchestrator flagged "Implementation Deadline Missed"

### Cycle 26 (Critical Re-evaluation - Athena)
- **Action**: Hired two fresh evaluators for BLIND assessment (no prior reports)
- **Evaluator 1 (Code Quality Auditor)**:
  - Hash function: ✅ PASS (portable and deterministic)
  - Correctness: ✅ PASS (all operations logically correct)
  - CRITICAL ISSUE 1: File count violation (delete creates `.tmp` files → 21 files)
  - CRITICAL ISSUE 2: O(n^2) insert complexity (15-21s estimated for 100K ops)
  - Recommendation: ❌ NO-GO
- **Evaluator 2 (Performance Tester)**:
  - Build: ✅ PASS, Sample test: ✅ PASS, Memory: ✅ PASS (1.3-1.8 MiB)
  - CRITICAL ISSUE 1: CMakeLists.txt doesn't apply -O2 in OJ build → 18.6s (FAILS)
  - CRITICAL ISSUE 2: O(n^2) insert performance → 15.2s (borderline), 16.9s (insert-heavy, FAILS)
  - Measured worst case: 129s for same-key inserts (CATASTROPHIC)
  - Recommendation: ❌ NO-GO
- **Key Findings**:
  1. CMakeLists.txt missing -O2 flag for OJ build process
  2. Delete operation creates temporary files (violates 20-file limit)
  3. Insert operation has O(n^2) complexity (too slow for 100K ops)
- **Decision**: M5 NOT READY - Define M5.1 to fix critical blockers
- **Key Lesson**:
  - Cycle 25 evaluators UNDERESTIMATED risks (didn't test OJ build, didn't test worst cases)
  - Fresh blind evaluation catches issues familiar evaluators miss
  - "Tight margin" (9%) is NOT acceptable when multiple risks compound
  - Always test actual OJ build process, not just local optimized build

### Cycle 27-29 (M5.1.4 Execution - Bloom Filter Catastrophe - Ares/Athena)
- **Milestone**: M5.1.4 (Add Bloom Filter) - 3 cycles
- **Outcome**: COMPLETE FAILURE - Performance worse than before
- **What Happened**:
  - Elena implemented bloom filter (8ed1074)
  - Maya fixed delete memory bloat (37c1bee)
  - Felix tested: ALL TESTS FAILED (78-379s vs 16s limit)
- **Root Cause**: Wrong architectural direction
  - Single-file architecture has fundamental I/O bottleneck
  - Bloom filter doesn't solve the core problem (file contention)
  - Find operations still require full file scan (no index)
- **Key Lessons**:
  1. **Architectural issues cannot be fixed with optimizations** - need fundamental redesign
  2. **Roadmap already said the answer** - "20-file architecture is required (not 1 file)"
  3. **Single-file approach (M5.1.3, M5.1.4) was wrong from the start**
  4. **Test incrementally** - Felix found catastrophic issues only after 3 cycles of work
- **Decision**: Abandon single-file approach, switch to multi-bucket architecture (M5.2)

### Cycle 27 (M5.1 Execution Failure - Ares/Athena)
- **Milestone**: M5.1 (Fix OJ Submission Blockers) - 3 cycles, 3 fixes
- **Outcome**: COMPLETE FAILURE - Ares did zero work
- **Investigation**: Lucas's blind audit revealed:
  - Ares workspace: Empty (created but no files, no notes)
  - Git commits: None
  - Code changes: None (all 3 fixes unimplemented)
  - Timeline: 3 cycles consumed with zero output
- **OJ Feedback** (Issue #30 from HUMAN):
  - Testpoint 28: Time Limit Exceeded (Subtask 9: Synthesized test 2)
  - Confirms performance issues are real on OJ platform
- **Root Cause Analysis**:
  - M5.1 was TOO BROAD with 3 distinct fixes (trivial → complex)
  - Each fix requires different skills and testing approaches
  - Monolithic milestone creates execution risk
- **Decision**: Break M5.1 into 3 focused sub-milestones (M5.1.1, M5.1.2, M5.1.3)
- **Key Lesson**:
  - Even "small" milestones can be too broad if they mix concerns
  - Break work into SINGLE-PURPOSE milestones
  - Start with easiest fix to build momentum and reduce risk
  - One clear objective per milestone > multiple objectives bundled together
