# Project Roadmap - File Storage (Problem 015)

## Project Goal
Implement a high-quality file-based key-value database for ACMOJ Problem 2545 that handles insert/delete/find operations with strict memory constraints (5-6 MiB).

## Current Status
- **Phase**: Initial Planning
- **Date**: 2026-02-25
- **Submission Budget**: 7 attempts remaining

## Milestones

### M1: Initial Setup and Basic Infrastructure
**Status**: Pending
**Estimated Cycles**: TBD
**Description**: Create project structure, CMakeLists.txt, .gitignore, and basic main.cpp skeleton

### M2: Core Data Structure Implementation
**Status**: Pending
**Estimated Cycles**: TBD
**Description**: Implement the file-based storage engine with appropriate data structure (likely B+ tree or similar)

### M3: Command Processing and I/O
**Status**: Pending
**Estimated Cycles**: TBD
**Description**: Implement insert, delete, and find commands with proper parsing and output formatting

### M4: Testing and Optimization
**Status**: Pending
**Estimated Cycles**: TBD
**Description**: Test against sample input, optimize for memory/time constraints, verify compilation process

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
(Will be updated as project progresses)
