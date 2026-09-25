# ryuzaki - Universal data search engine

An asynchronous HTTP search service that searches across multiple local data sources.

The client submits a search request in a single call, receives a `task_id`, and polls the results until the task is completed. Sources can be added simply by creating a directory under `data/` and implementing the corresponding module inherited from `ISource`.

* **Language:** C++20
* **HTTP:** [Drogon](https://github.com/drogonframework/drogon)
* **Task storage:** in-memory (`RamStorage`); inherit from `IStorage` if you need to move task storage elsewhere.
* **Logging:** spdlog (async)
* **Testing:** GoogleTest

## Features

* Fans out a request across all sources in `data/` and aggregates the results
* Asynchronous task queue and worker pool (`std::thread::hardware_concurrency()`)
* `entity → table/column` mapping defined in JSON, with no code changes required
* Query normalization: `lower`, `onlynumeric`, `none`, `default`
* Sources are opened in read-only mode

## Requirements

* CMake ≥ 3.16, a compiler with C++20 support
* Boost (system, json, locale, uuid)
* spdlog, jsoncpp, OpenSSL, zlib
* Drogon ≥ 1.9
* GoogleTest

### Installing Dependencies (Arch Linux)

```bash
bash install-arch.sh      # pacman: base-devel cmake boost jsoncpp spdlog ...
```

## Building

```bash
./build.sh                  # Debug
./build.sh --release        # Release
./build.sh --test           # Debug + tests
./build.sh --run            # build and run the server
./build.sh --test --run     # build and run tests (ctest)
```

`build.sh` cleans `./build` on every run.

Manual equivalent:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
```

Binaries: `./build/ryuzaki`, `./build/ryuzaki_tests`

### Configuration — `config.json`

```json
{
    "ip": "0.0.0.0",
    "port": 10511
}
```

If the file is missing, the default values from `config.hpp` are used (`0.0.0.0:10511`).

There are no environment variables. The `data/` directory is required; the process will terminate with an error if it is missing.

## Data Sources

Each source is stored in a separate directory under `data/`:

```
data/
└── my_source/
    ├── config.json     # type and metadata
```

`data/<name>/config.json`:

```json
{
    "type": "sqlite",   // source type is determined by this field
    "metadata": {       // source-specific metadata
        "filename": "example.db",
        "mappingFile": "mapping.json"
    }
}
```

### Implemented Source Types

#### SQLiteSource

```
    ├── mapping.json    # entity → table/column mapping
    └── my_source.db
```

`data/<name>/mapping.json`:

```json
{
  "entities": {
    "email": {
      "mappings": [ { "table": "users", "column": "email" } ],
      "normalizer": "none"
    },
    "card_number": {
      "mappings": [ { "table": "users", "column": "card_number" } ]
    }
  }
}
```

* `mappings` — required array of `table`/`column` pairs
* `normalizer` — optional; if not specified, the value from `src/constants/normalizers.hpp` is used:

| entity                                | normalizer                  |
| ------------------------------------- | --------------------------- |
| `default`, `name`, `email`, `address` | `lower`                     |
| `phone`, `card_number`                | `onlynumeric`               |
| others                                | `default` (trim whitespace) |

A ready-to-use example source is available in `data_example/`.

## API

All endpoints use `GET`.

| Path                                       | Response                                                    |
| ------------------------------------------ | ----------------------------------------------------------- |
| `/ping`                                    | `text/plain`: `pong`                                        |
| `/search/start?entity=<name>&query=<text>` | `200` JSON with `task_id`, or `400` if parameters are empty |
| `/search/results/{taskId}`                 | JSON with results, `404` if the task is not found           |

### Example

```bash
# 1. Create a task
curl 'http://127.0.0.1:10511/search/start?entity=address&query=Москва'
# {"status":"ok","task_id":"8f14e45f-..."}

# 2. Poll until status == 2
curl 'http://127.0.0.1:10511/search/results/8f14e45f-...'
```

Response from `/search/results/{taskId}`:

```json
{
  "task_id": "8f14e45f-...",
  "status": 2,
  "results": [
    {
      "source": "test",
      "data": [ { "id": 1, "email": "...", "address": "..." } ]
    }
  ]
}
```

`status`: `0` — Pending, `1` — InProgress, `2` — Completed, `3` — Failed.

When `status == 3`, the response also contains an `error` field.

Supported `entity` values are the keys defined in the `mapping.json` files of each source. Examples include: `last_name`, `first_name`, `patronymic`, `address`, `card_number`, `last_ip`, `email`.

## Tests

```bash
./build.sh --test --run
# or
ctest --test-dir build --output-on-failure
./build/ryuzaki_tests
```

## Scripts

```bash
# Generate a synthetic SQLite database (1,000,000 records by default)
python scripts/generate_test_db.py -c 1000000 -o data/test/test.db

# Load test: 1,000 requests, concurrency 50
# (the server must be running at http://127.0.0.1:10511)
python scripts/benchmark.py
```

The benchmark prints RPS and p50/p95/p99 latency metrics; `aiohttp` is required.

The generated data is synthetic.

## License

Apache License 2.0 — see [LICENSE](LICENSE).
