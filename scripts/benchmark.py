#!/usr/bin/env python3

# AI CREATED SCRIPT

import asyncio
import aiohttp
import time
import statistics
from dataclasses import dataclass


BASE_URL = "http://127.0.0.1:10511"

START_URL = (
    f"{BASE_URL}/search/start"
    "?query=%D0%9C%D0%BE%D1%81%D0%BA%D0%B2%D0%B0,%20"
    "%D1%83%D0%BB.%20%D0%A6%D0%B5%D0%BD%D1%82%D1%80%D0%B0%D0%BB%D1%8C%D0%BD%D0%B0%D1%8F,%20"
    "%D0%B4.%20142"
    "&entity=address"
)

RESULTS_URL = f"{BASE_URL}/search/results"


# Настройки
TOTAL_REQUESTS = 1000
CONCURRENCY = 50
POLL_INTERVAL = 0.05
REQUEST_TIMEOUT = 30


@dataclass
class Stats:
    completed: int = 0
    failed: int = 0
    total_time: float = 0
    latencies: list[float] | None = None

    def __post_init__(self):
        self.latencies = []


async def start_task(session: aiohttp.ClientSession):
    start = time.perf_counter()

    try:
        async with session.get(START_URL) as response:
            if response.status != 200:
                return None, None

            data = await response.json()

            task_id = data.get("task_id")

            elapsed = time.perf_counter() - start

            return task_id, elapsed

    except Exception:
        return None, None


async def wait_for_result(
    session: aiohttp.ClientSession,
    task_id: str,
):
    url = f"{RESULTS_URL}/{task_id}"

    while True:
        async with session.get(url) as response:
            if response.status == 404:
                return False

            if response.status != 200:
                return False

            data = await response.json()

            status = data.get("status")

            # TaskStatus:
            # Pending = 0
            # InProgress = 1
            # Completed = 2
            # Failed = 3

            if status == 2:
                return True

            if status == 3:
                return False

        await asyncio.sleep(POLL_INTERVAL)


async def worker(
    session: aiohttp.ClientSession,
    semaphore: asyncio.Semaphore,
    stats: Stats,
):
    async with semaphore:
        pipeline_start = time.perf_counter()

        task_id, start_latency = await start_task(session)

        if not task_id:
            stats.failed += 1
            return

        success = await wait_for_result(session, task_id)

        elapsed = time.perf_counter() - pipeline_start

        if success:
            stats.completed += 1
            stats.latencies.append(elapsed)
        else:
            stats.failed += 1


async def main():
    timeout = aiohttp.ClientTimeout(
        total=REQUEST_TIMEOUT
    )

    connector = aiohttp.TCPConnector(
        limit=CONCURRENCY
    )

    stats = Stats()

    semaphore = asyncio.Semaphore(CONCURRENCY)

    async with aiohttp.ClientSession(
        timeout=timeout,
        connector=connector,
    ) as session:

        start = time.perf_counter()

        tasks = [
            worker(session, semaphore, stats)
            for _ in range(TOTAL_REQUESTS)
        ]

        await asyncio.gather(*tasks)

        total_time = time.perf_counter() - start

    print("\n========== RESULTS ==========")

    print(f"Total requests: {TOTAL_REQUESTS}")
    print(f"Concurrency: {CONCURRENCY}")

    print(f"Completed: {stats.completed}")
    print(f"Failed: {stats.failed}")

    print(f"Total time: {total_time:.2f}s")

    print(
        f"Pipeline RPS: "
        f"{stats.completed / total_time:.2f}"
    )

    if stats.latencies:
        latencies = sorted(stats.latencies)

        print(
            f"Latency avg: "
            f"{statistics.mean(latencies):.3f}s"
        )

        print(
            f"Latency p50: "
            f"{latencies[int(len(latencies) * 0.50)]:.3f}s"
        )

        print(
            f"Latency p95: "
            f"{latencies[int(len(latencies) * 0.95)]:.3f}s"
        )

        print(
            f"Latency p99: "
            f"{latencies[int(len(latencies) * 0.99)]:.3f}s"
        )


if __name__ == "__main__":
    asyncio.run(main())