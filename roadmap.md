# Project Roadmap - File Storage (Problem 015)

## Project Goal
Implement a high-quality file-based key-value database for ACMOJ Problem 2545 that handles insert/delete/find operations with strict memory constraints (5-6 MiB).

## Current Status
- **Phase**: OJ Failure Investigation & Fix
- **Date**: 2026-02-25 (Cycle 24)
- **Submission Budget**: 5 attempts remaining (2 used: 750119, 750120)
- **State**: OJ submissions failed - hash portability issue identified

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
**Status**: 🔄 IN PROGRESS
**Estimated Cycles**: 2
**Description**: Replace non-portable std::hash with deterministic hash function
**Requirements**:
1. Implement portable hash function (FNV-1a or polynomial rolling hash)
2. Replace std::hash usage in bucket_manager.cpp:13
3. Verify hash distribution quality (test with diverse keys)
4. Verify persistence still works correctly
5. Run all existing tests (sample, 100K stress, edge cases)
6. Ensure memory and time constraints still met
**Success Criteria**:
- ✅ Hash function produces identical results across platforms
- ✅ All local tests pass (sample, stress, edge cases)
- ✅ Memory ≤ 6 MiB, Time ≤ 16s on 100K operations
- ✅ Ready for re-submission to OJ

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
