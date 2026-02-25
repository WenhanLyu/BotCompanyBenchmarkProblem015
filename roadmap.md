# Project Roadmap - File Storage (Problem 015)

## Project Goal
Implement a high-quality file-based key-value database for ACMOJ Problem 2545 that handles insert/delete/find operations with strict memory constraints (5-6 MiB).

## Current Status
- **Phase**: Optimization Required
- **Date**: 2026-02-25 (Cycle 3)
- **Submission Budget**: 7 attempts remaining
- **Critical Issue**: Implementation fails at maximum scale (100K operations)

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
**Status**: In Progress
**Allocated Cycles**: 8
**Description**: Optimize implementation to handle 100K operations within resource limits
**Problem Identified**:
- Current implementation: 6.53 MiB for 100K operations (exceeds 6 MiB limit)
- Current implementation: 41.19s for 100K operations (exceeds 16s limit)
- Root cause: Loading entire buckets into memory for every operation
**Required Changes**:
- Implement streaming-based processing (don't load full buckets)
- Process bucket files entry-by-entry or in small chunks
- Optimize insert: stream through bucket to check duplicates, append if new
- Optimize find: stream through bucket, collect matching values, sort
- Optimize delete: stream through bucket to temporary file, rename when done
**Success Criteria**:
- 100K operations use ≤ 6 MiB memory (tested with /usr/bin/time -l)
- 100K operations complete in ≤ 16 seconds
- All existing tests still pass (correctness maintained)
- Sample test still produces correct output

### M4: Final Testing and Submission Prep
**Status**: Pending
**Estimated Cycles**: 2
**Description**: Final validation before submission
- Verify 100K operations pass all constraints
- Test with adversarial hash collision scenarios
- Complete self-review
- Verify compilation process matches OJ requirements
- Ready for external OJ evaluation

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
- **Next Step**: Optimize for streaming/chunked processing instead of loading full buckets
