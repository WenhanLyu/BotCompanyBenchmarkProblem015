# Project Roadmap - File Storage (Problem 015)

## Project Goal
Implement a high-quality file-based key-value database for ACMOJ Problem 2545 that handles insert/delete/find operations with strict memory constraints (5-6 MiB).

## Current Status
- **Phase**: Initial Planning
- **Date**: 2026-02-25
- **Submission Budget**: 7 attempts remaining

## Architecture Decision

**Chosen Approach**: Hash-based bucketing with 20 files (Lucas's design)
**Rationale**: Simpler implementation, adequate performance (O(n/20)), lower risk for limited submission budget

## Milestones

### M1: Working Skeleton with Insert/Find
**Status**: In Progress
**Allocated Cycles**: 6
**Description**: Build minimal working system that passes sample test
- CMakeLists.txt and .gitignore for build system
- main.cpp with command parser (insert/find only)
- BucketManager class with hash bucketing
- Binary file I/O for 20 bucket files
- Compiles to `code` executable
- Passes sample test from README
**Success Criteria**:
- `make` produces working `code` executable
- Sample input → correct output
- No memory leaks (valgrind check)

### M2: Complete Implementation with Delete
**Status**: Pending
**Estimated Cycles**: 4
**Description**: Add delete operation and handle all edge cases
- Implement lazy deletion with tombstones
- Handle "delete non-existent" gracefully
- Test with comprehensive cases

### M3: Optimization and Hardening
**Status**: Pending
**Estimated Cycles**: 3
**Description**: Optimize performance and ensure robustness
- Memory usage verification (< 5 MiB)
- Performance testing with 100K operations
- Edge case handling (empty finds, duplicates, persistence)
- Build system validation

### M4: Final Testing and Submission Prep
**Status**: Pending
**Estimated Cycles**: 2
**Description**: Final validation before submission
- Complete self-review
- Verify compilation process matches OJ requirements
- Test persistence across multiple runs
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

### Cycle 1 (Planning)
- **Decision**: Chose hash-based bucketing over B+ tree for simplicity
- **Research**: Both Lucas and Sophia provided excellent analysis
- **Strategy**: Prioritize working code over optimal complexity for limited submission budget
