# Synthetics Configuration System

A comprehensive C++ configuration management system for financial data processing pipelines, designed for derivatives and index data handling across multiple markets.

## Overview

This project provides a robust, hierarchical configuration framework that maps JSON-based settings to strongly-typed C++ structs. It supports processing of financial instruments including options, futures, and index data.

### Upcoming Features

**File Generation System** (Coming in the next few weeks)
- The system will read configuration files and automatically generate corresponding data processing files
- Dynamic file creation based on configuration parameters
- Automatic schema generation and initialization
- Extensible architecture for custom file types

## Project Structure

```
synthetics/
├── config.cpp         # Main configuration parser and implementation
├── config.json        # JSON configuration file with all settings
└── README.md          # This file
```

## Key Features

- **JSON-based Configuration**: Human-readable JSON configuration file
- **Type-Safe Parsing**: Automatic mapping to strongly-typed C++ structs
- **Hierarchical Organization**: 12 major configuration domains
- **Flexible Loading**: Load from file or in-memory JSON string
- **Default Values**: Sensible defaults for all configuration parameters
- **Error Handling**: Comprehensive error reporting and validation

## Configuration Domains

### 1. **Data Paths** (`data_paths`)
Defines root directories for data storage and output.
- `derivatives_root`: Path to derivatives (futures/options) data
- `spot_root`: Path to spot/index data
- `export_root`: Output directory for processed data
- `log_root`: Directory for application logs

### 2. **Data Scope** (`data_scope`)
Specifies what data to process.
- `underlyings`: List of assets/symbols to process (e.g., ["ASSET1", "ASSET2"])
- `date_from`, `date_to`: Date range for processing
- `instrument_classes`: Types of instruments (options, futures, index, etc.)
- `expiry_limit`: Maximum number of expiries to keep

### 3. **Symbol Registry** (`symbol_registry`)
Maps logical asset names to exchange-specific symbols.
- Supports nested mappings for different instrument types
- Example: ASSET1 → options_symbol, futures_symbol, index_symbol

### 4. **Symbol Matching** (`symbol_matching`)
Controls symbol normalization and matching behavior.
- `options_mode`, `futures_mode`, `index_mode`: Matching strategies
- `is_case_sensitive`: Case sensitivity for symbol matching
- `trim_whitespace`: Remove leading/trailing spaces

### 5. **Preprocessing** (`preprocessing`)
Data cleaning and aggregation settings.
- `forward_fill`, `backward_fill`: Fill missing values
- `ignore_empty_files`: Skip zero-row datasets
- `merge_daily_outputs`: Combine daily partitions

### 6. **Acceleration** (`acceleration`)
GPU and performance optimization settings.
- `enable_gpu`: Toggle GPU kernel acceleration

### 7. **Logger** (`logger`)
Logging configuration.
- `stdout_level`, `file_log_level`: Log verbosity levels
- `log_template`: Log message format
- `timestamp_format`: Date/time format in logs

### 8. **Export** (`export`)
Output file format configuration.
- `file_format`: Output format (parquet, CSV, etc.)
- `codec`: Compression algorithm (snappy, lz4, etc.)

### 9. **Stream Logging** (`stream_logging`)
Real-time data stream logging (optional).
- `is_enabled`: Enable/disable stream logging
- `stream_log_root`: Directory for stream logs
- `output_formats`: Supported output formats (arrow_ipc, jsonl)

### 10. **Execution** (`execution`)
Advanced performance tuning with 41 parameters.
- **Parallelism**: `enable_parallelism`, `parallelize_days`, `parallelize_assets`, etc.
- **Worker Management**: `global_worker_cap`, `day_worker_cap`, `file_worker_cap`, etc.
- **Memory Management**: `low_memory_mode`, `use_memory_controller`
- **Chunk Sizes**: `io_chunk_size`, `greeks_block_size`, `tte_block_size`, etc.
- **Specialized Engines**: Fill engine, monthly processing, futures, greeks, TTE (time-to-expiry), synthetic futures

### 11. **Post-Compute** (`post_compute`)
Post-processing transformations.
- `compute_synthetic_futures`: Calculate synthetic futures prices
- `recompute_theoretical_greeks`: Recalculate option Greeks

### 12. **Market Constants** (`market_constants`)
Market-wide constants and trading information.
- `valid_underlyings`: List of supported assets
- `symbol_exceptions`: Assets with special naming or formatting rules
- `expiry_cutoff_time`: Expiry time as [hours, minutes, seconds]
- Month mapping tables: Calendar, numeric, and alpha representations
- `trading_schedule`: Market hours and trading sessions per year
- `exchange_holidays`: List of non-trading dates

## Building and Running

### Compile
```bash
g++ -std=c++17 config.cpp -o config_parser
```

### Run
```bash
# Load default config.json
./config_parser

# Load custom config file
./config_parser /path/to/config.json
```

## Configuration File Format

The configuration is stored in JSON format with the following structure:

```json
{
  "data_paths": { ... },
  "data_scope": { ... },
  "symbol_registry": { ... },
  "symbol_matching": { ... },
  "preprocessing": { ... },
  "acceleration": { ... },
  "logger": { ... },
  "export": { ... },
  "stream_logging": { ... },
  "execution": { ... },
  "post_compute": { ... },
  "market_constants": { ... }
}
```

See `config.json` for a complete example with all supported fields.

## Usage in C++ Code

```cpp
#include "config.cpp"

int main() {
    config cfg;
    
    // Load from file
    if (!cfg.load_from_file("config.json")) {
        std::cerr << "Failed to load config" << std::endl;
        return 1;
    }
    
    // Access configuration
    std::string output_dir = cfg.data_paths_config->export_root;
    int workers = cfg.execution_config->global_worker_cap;
    std::vector<std::string> assets = cfg.data_scope_config->underlyings;
    
    return 0;
}
```

## Dependencies

- **Standard C++ Library**: iostream, string, fstream, vector, map
- **nlohmann/json**: For JSON parsing
  - Install via package manager: `apt-get install nlohmann-json3-dev`
  - Or download from: https://github.com/nlohmann/json

## Error Handling

The configuration system provides detailed error messages for:
- Missing or unreadable JSON files
- Invalid JSON syntax
- Missing required fields
- Type mismatches

All errors are printed to `stderr` with context information.

## Performance Tuning

The `execution` section contains 41 parameters for fine-tuning performance:

- **CPU-bound workloads**: Increase worker counts and reduce memory_efficient mode
- **Memory-constrained**: Enable `low_memory_mode` and reduce chunk sizes
- **Multi-core systems**: Enable parallelism across days, assets, and file reading
- **GPU acceleration**: Set `acceleration.enable_gpu = true` with CUDA toolkit

## Examples

### Minimal Configuration
```json
{
  "data_paths": {
    "derivatives_root": "./data/derivatives",
    "spot_root": "./data/spot",
    "export_root": "./output"
  },
  "data_scope": {
    "underlyings": ["ASSET1", "ASSET2"],
    "date_from": "2023-01-01",
    "date_to": "2023-12-31"
  }
}
```

### High-Performance Configuration
```json
{
  "execution": {
    "enable_parallelism": true,
    "global_worker_cap": 32,
    "parallelize_days": true,
    "parallelize_assets": true,
    "parallel_file_io": true,
    "low_memory_mode": false
  }
}
```

## Contributing

When modifying configuration:
1. Update both the struct definition and JSON parsing logic
2. Maintain naming consistency with JSON keys
3. Document new parameters in this README
4. Test with various JSON inputs
5. Ensure backward compatibility where possible
6. Keep documentation market-agnostic with generic examples

## Roadmap

**Phase 1 (Current)**: Configuration framework and JSON parsing

**Phase 2 (Coming in next few weeks)**: 
- File generation system
- Dynamic schema creation
- Automatic file initialization based on configuration
- Support for custom file types and formats

**Phase 3 (Future)**:
- Configuration validation schemas
- Migration tools for configuration updates
- Web UI for configuration management
- Configuration versioning and rollback

## License

(Add your license information here)

## Contact

(Add contact information here)
