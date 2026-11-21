"""
Python Lock-Free Latency Benchmark Visualizer
Computes latency distribution percentiles (p50, p99, p99.9) for inter-thread communication using standard library.
"""

import random

def analyze_mock_latency_profile():
    random.seed(42)
    # Model 10,000 transactions: lognormal micro-latency profile
    latencies = sorted([random.lognormvariate(2.5, 0.4) for _ in range(10000)])

    p50 = latencies[int(len(latencies) * 0.50)]
    p95 = latencies[int(len(latencies) * 0.95)]
    p99 = latencies[int(len(latencies) * 0.99)]
    p999 = latencies[int(len(latencies) * 0.999)]

    print(f"[*] Lock-Free SPSC Ring Buffer Synthetic Latency Profile:")
    print(f"    p50:   {p50:.1f} ns")
    print(f"    p95:   {p95:.1f} ns")
    print(f"    p99:   {p99:.1f} ns")
    print(f"    p99.9: {p999:.1f} ns")

if __name__ == "__main__":
    analyze_mock_latency_profile()
