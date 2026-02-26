# Blind Performance Evaluation: Commit 6d93bcb

**Commit:** 6d93bcb40dff3527df527e38bbfa0df9b00f68f9
**Date:** 2026-02-26
**Author:** Elena
**Description:** Implement bounded in-memory index with LRU eviction (Issue #42)

## Evaluation Methodology

This is a **blind evaluation** - performance tests were conducted without prior knowledge of expected results or previous implementation performance. The goal is to objectively measure the current implementation's characteristics.

### Test Environment
- Platform: macOS (Darwin 24.6.0)
- Compiler: g++ with -std=c++17 -O2 optimization
- Test Date: 2026-02-26

### Test Workloads

Three 100,000-operation workloads were executed:

1. **Random Mix** (`test_random_100k.txt`)
   - Balanced distribution: ~34% insert, ~34% delete, ~31% find
   - Key space: 10,000 unique keys (key0 - key9999)
   - Value range: 1 - 1,000,000

2. **Insert-Heavy** (`test_insert_heavy_100k.txt`)
   - Insert-dominated: ~90% insert, ~6% find, ~4% delete
   - High insertion pressure with minimal deletions

3. **Collision-Heavy** (`test_collision_100k.txt`)
   - Balanced operations: ~36% insert, ~34% delete, ~30% find
   - Limited key space to force collisions and duplicate checking

## Performance Results

### 1. Random Mix Workload (100K ops)

**Execution Time:**
- Real time: 76.82 seconds
- User CPU: 71.51 seconds
- System CPU: 4.81 seconds
- **Throughput: 1,301 ops/sec**

**Resource Usage:**
- Data file size: 419 KB
- Output lines: 33,164 (find results)
- File count: 1 (data.bin)

**Analysis:**
- CPU-bound workload (93% user CPU)
- Reasonable I/O overhead (6% system CPU)
- Compact data storage

### 2. Insert-Heavy Workload (100K ops)

**Execution Time:**
- Real time: 203.38 seconds (3:23.38)
- User CPU: 193.62 seconds
- System CPU: 8.80 seconds
- **Throughput: 492 ops/sec**

**Resource Usage:**
- Output lines: 4,974 (minimal find results)
- File count: 1 (data.bin)

**Analysis:**
- **2.65x slower than random mix**
- Higher system CPU time (4.3% vs 6.3%)
- Insert operations dominate runtime
- Significantly lower throughput under insert pressure

**⚠️ Performance Concern:**
Insert-heavy workload shows **substantial performance degradation** compared to balanced operations. This suggests:
- Possible O(n) duplicate checking overhead
- File I/O bottleneck on repeated inserts
- Memory or index lookup inefficiency

### 3. Collision-Heavy Workload (100K ops)

**Execution Time:**
- Real time: 77.85 seconds (1:17.85)
- User CPU: 72.50 seconds
- System CPU: 4.86 seconds
- **Throughput: 1,285 ops/sec**

**Resource Usage:**
- Data file size: 439 KB
- Output lines: 33,019 (find results)
- File count: 1 (data.bin)

**Analysis:**
- Performance similar to random mix (within 1.3%)
- Collision handling does not significantly impact performance
- Duplicate detection appears efficient

## Cross-Workload Comparison

| Metric | Random Mix | Insert-Heavy | Collision |
|--------|-----------|--------------|-----------|
| Real Time (s) | 76.82 | 203.38 | 77.85 |
| Throughput (ops/s) | 1,301 | 492 | 1,285 |
| User CPU (s) | 71.51 | 193.62 | 72.50 |
| System CPU (s) | 4.81 | 8.80 | 4.86 |
| Data Size (KB) | 419 | (N/A) | 439 |
| Output Lines | 33,164 | 4,974 | 33,019 |

**Key Observations:**

1. **Insert Performance Issue**: Insert-heavy workload is **2.65x slower** than balanced workloads
   - This is the **primary performance bottleneck** identified
   - Suggests O(n) behavior during inserts despite in-memory index

2. **Collision Handling**: Efficient collision/duplicate handling
   - Collision workload performs nearly identically to random mix
   - LRU cache appears to handle collisions well

3. **Consistent System CPU**: System time remains <7% across all workloads
   - File I/O is reasonably efficient
   - Not I/O-bound

4. **Memory Efficient**: Single file storage keeps disk usage minimal
   - All workloads produce ~400-440 KB data files
   - Well under file count constraints

## Architecture Analysis (from commit message)

The implementation uses:
- **NUM_BUCKETS = 1**: Single data.bin file (down from 5000)
- **MAX_INDEX_ENTRIES = 15000**: Bounded in-memory cache
- **LRU eviction**: Least-recently-used eviction policy
- **Memory estimate**: ~750 KB (under 6 MiB budget)

## Critical Findings

### ✅ Strengths
1. **Collision handling**: Excellent performance with high collision rates
2. **Memory efficiency**: Bounded cache keeps memory usage controlled
3. **File constraints**: Single file meets file count limits
4. **Stable performance**: Random and collision workloads show consistent behavior

### ⚠️ Performance Concerns
1. **Insert bottleneck**: 2.65x slowdown on insert-heavy workload is **significant**
   - Throughput drops from 1,301 to 492 ops/sec
   - This may fail performance requirements for write-heavy scenarios

2. **Potential O(n) behavior**: Despite in-memory index, inserts show degradation
   - Suggests either:
     - Cache eviction causing frequent file scans
     - Index lookup not O(1) in practice
     - File rewrite overhead on inserts

3. **Scalability question**: If 100K ops takes 3.5 minutes with 90% inserts, larger workloads may be problematic

## Recommendations for Investigation

1. **Profile insert operation**: Identify where time is spent during inserts
   - Is it duplicate checking? File writing? Index updates? LRU maintenance?

2. **Test cache hit rate**: Measure how often index cache hits vs. file scans
   - MAX_INDEX_ENTRIES = 15000 may be insufficient for insert-heavy workloads

3. **Consider batch writes**: If file rewriting is the bottleneck, batch or buffer inserts

4. **Verify O(1) complexity**: Ensure index lookups are truly constant time

## Conclusion

Commit 6d93bcb demonstrates **mixed performance**:
- ✅ Excellent for balanced and collision-heavy workloads (~1,300 ops/sec)
- ⚠️ Poor for insert-heavy workloads (492 ops/sec, 2.65x degradation)

The **insert performance bottleneck** is the primary concern and should be investigated before accepting this implementation for production use.

**Overall Assessment**: Implementation meets file constraints and memory limits, but insert performance requires optimization before this can be considered production-ready for write-heavy applications.
