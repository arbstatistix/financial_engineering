/**
 * ===========================
 * Synthetics Configuration System
 * ===========================
 * 
 * A comprehensive configuration management system for financial data processing
 * pipelines. Provides JSON-based configuration with automatic mapping to
 * strongly-typed C++ structs for derivatives and index data processing across
 * multiple markets.
 * 
 * Features:
 *  - Hierarchical configuration structure with 12 configuration domains
 *  - JSON file and string-based loading
 *  - Type-safe struct mapping with defaults
 *  - Comprehensive error handling and validation
 *  - Supports complex execution tuning with 41+ parameters
 */

/*
 * ===========================
 * Standard & Third-Party Includes
 * ===========================
 * iostream      : standard input/output for logging and errors
 * string        : std::string for text manipulation
 * fstream       : file stream for JSON file reading
 * vector        : std::vector for dynamic arrays
 * map           : std::map for key-value storage and symbol registries
 * nlohmann/json : JSON parsing library for configuration deserialization
 */
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
 * ===========================
 * Forward Declarations
 * ===========================
 * Forward-declaring configuration structs allows:
 *  - separation of interface and implementation
 *  - pointer-based ownership inside config class
 *  - faster compilation and cleaner dependency graph
 */
struct data_paths;
struct data_scope;
struct symbol_registry;
struct symbol_matching;
struct preprocessing;
struct acceleration;
struct logger;
struct export_config;
struct stream_logging;
struct execution;
struct post_compute;
struct market_constants;

/*
 * ===========================
 * Central Configuration Class
 * ===========================
 * Acts as a top-level container for all configuration domains.
 * Each pointer corresponds to a logically isolated configuration module.
 *
 * Design Rationale:
 *  - Pointers enable optional configuration sections and lazy initialization
 *  - JSON parsing is centralized and delegated to parse_json()
 *  - Supports partial configuration loading with NULL for missing sections
 *  - Memory management: caller is responsible for cleanup
 *
 * Public API:
 *  - load_from_file(): Load configuration from JSON file on disk
 *  - load_from_string(): Load configuration from in-memory JSON string
 *  - to_flat_map(): Convert hierarchical config to flat key-value map (if implemented)
 */
class config {
public:
    // Configuration pointers for each major domain
    // Organized by functional area: paths, data scope, symbol handling, 
    // processing rules, execution tuning, and market constants
    
    data_paths* data_paths_config;              // File system paths for data input/output
    data_scope* data_scope_config;              // Data extraction scope and date ranges
    symbol_registry* symbol_registry_config;    // Asset-to-symbol mappings per exchange
    symbol_matching* symbol_matching_config;    // Symbol matching behavior and rules
    preprocessing* preprocessing_config;        // Data cleaning and aggregation settings
    acceleration* acceleration_config;          // GPU and hardware acceleration toggles
    logger* logger_config;                      // Logging verbosity and formatting
    export_config* export_config_ptr;           // Output file format and compression
    stream_logging* stream_logging_config;      // Real-time data stream logging
    execution* execution_config;                // Advanced parallelism and worker tuning
    post_compute* post_compute_config;          // Post-processing and quantitative transformations
    market_constants* market_constants_config;  // Market timing, holidays, and constants

    config() = default;

    /*
     * Load configuration from a JSON file on disk.
     * 
     * Parameters:
     *  file_name: Path to JSON configuration file
     * 
     * Returns:
     *  true if configuration loaded and parsed successfully
     *  false if file cannot be opened or JSON parsing fails
     * 
     * Error Handling:
     *  - Logs file open failures to stderr
     *  - Logs JSON syntax errors with context to stderr
     *  - Partial failure on missing required fields handled gracefully
     */
    bool load_from_file(const std::string& file_name);

    /*
     * Load configuration from an in-memory JSON string.
     * 
     * Parameters:
     *  config_str: Valid JSON string containing configuration
     * 
     * Returns:
     *  true if configuration parsed successfully
     *  false if JSON parsing fails
     * 
     * Use Cases:
     *  - Unit testing without file I/O
     *  - Remote configuration injection
     *  - Dynamic runtime configuration overrides
     *  - Configuration from environment variables
     */
    bool load_from_string(const std::string& config_str);

    /*
     * Flatten hierarchical configuration to a flat key-value map.
     * Useful for debugging, logging, and flat configuration outputs.
     * 
     * Returns:
     *  Map of fully-qualified keys (e.g., "data_paths.export_root") to string values
     */
    std::map<std::string, std::string> to_flat_map() const;

private:
    /*
     * Core JSON parser and configuration loader.
     * Maps validated JSON fields into strongly-typed configuration structs.
     * 
     * Parameters:
     *  j: Parsed JSON object from nlohmann/json library
     * 
     * Returns:
     *  true if all recognized configuration sections parsed successfully
     *  false if critical parsing errors occur
     * 
     * Parsing Strategy:
     *  - Each configuration section is optional; NULL if missing
     *  - Default values applied at struct declaration level
     *  - Type coercion applied where needed (e.g., float to int)
     */
    bool parse_json(const json& j);
};



struct data_paths {
    /**
     * Paths Configuration Structure
     * =============================
     * Defines all file system root directories used for data input/output.
     * All paths are absolute or relative to the working directory.
     */
    
    std::string derivatives_root;  // Root directory for NFO (futures/options) tick data
    std::string spot_root;         // Root directory for spot/INDEX tick data
    std::string export_root;       // Output directory for processed/transformed data
    std::string log_root;          // Directory for application logs and diagnostics
};
/*
 * ===========================
 * Data Scope Configuration
 * ===========================
 * Defines the scope and range of data to be extracted and processed.
 * Controls which assets, date ranges, and instrument types are included.
 */
struct data_scope {
    std::vector<std::string> underlyings;      // Assets/symbols to process (market-agnostic)
    std::string date_from;                     // Start date for data extraction (YYYY-MM-DD)
    std::string date_to;                       // End date for data extraction (YYYY-MM-DD)
    std::vector<std::string> instrument_classes; // Types of instruments: options, futures, index
    int expiry_limit = 0;                      // Max expiries per underlying (0 = unlimited)
};

/*
 * ===========================
 * Symbol Registry Configuration
 * ===========================
 * Maps logical asset identifiers to exchange-specific trading symbols.
 * Supports multi-level nesting for different instrument types and exchanges.
 * Example: "ASSET1" -> {"options_symbol": "OPT_SYMBOL", "futures_symbol": "FUT_SYMBOL", "index_symbol": "IDX_SYMBOL"}
 */
struct symbol_registry {
    std::map<std::string, std::map<std::string, std::string>> mappings;  // Asset -> {symbol_type -> actual_symbol}
};

/*
 * ===========================
 * Symbol Matching Rules
 * ===========================
 * Controls symbol matching behavior, normalization, and lookup strategies.
 * Affects how instrument symbols are compared and standardized across the system.
 */
struct symbol_matching {
    std::string options_mode;      // Matching strategy for options: "exact", "prefix", "fuzzy", etc.
    std::string futures_mode;      // Matching strategy for futures
    std::string index_mode;        // Matching strategy for index instruments
    bool is_case_sensitive;        // Case-sensitive symbol comparison
    bool trim_whitespace;          // Remove leading/trailing whitespace from symbols
};

/*
 * ===========================
 * Preprocessing Rules
 * ===========================
 * Defines data quality and aggregation transformations applied during processing.
 * Controls how missing data is handled and how daily partitions are combined.
 */
struct preprocessing {
    bool backward_fill;         // Fill missing values with previous available value
    bool forward_fill;          // Fill missing values with next available value
    bool ignore_empty_files;    // Skip files with zero rows or no data
    bool merge_daily_outputs;   // Combine daily partitions into single output
};

/*
 * ===========================
 * Acceleration Configuration
 * ===========================
 * Hardware acceleration settings for GPU-based processing where available.
 * Requires compatible GPU and CUDA toolkit for effective operation.
 */
struct acceleration {
    bool enable_gpu;  // Enable GPU kernel acceleration (requires CUDA toolkit)
};

/*
 * ===========================
 * Logger Configuration
 * ===========================
 * Controls logging verbosity, output destinations, and formatting.
 * Supports separate log levels for console and file outputs.
 */
struct logger {
    std::string stdout_level;     // Console log level: DEBUG, INFO, WARNING, ERROR
    std::string file_log_level;   // File log level: DEBUG, INFO, WARNING, ERROR
    std::string log_template;     // Log message format string (e.g., "%(asctime)s - %(levelname)s - %(message)s")
    std::string timestamp_format; // Timestamp format string (e.g., "%Y-%m-%d %H:%M:%S")
};

/*
 * ===========================
 * Export Configuration
 * ===========================
 * Defines final output file format and compression strategy.
 * Parquet is recommended for performance (5-10x faster read/write, 3-5x smaller files vs CSV).
 */
struct export_config {
    std::string file_format;  // Output format: parquet, csv, arrow, etc.
    std::string codec;        // Compression algorithm: snappy, gzip, lz4, brotli, zstd
};

/*
 * ===========================
 * Stream Logging Configuration
 * ===========================
 * Optional real-time data stream logging for debugging and audit trails.
 * Captures intermediate data snapshots during processing pipelines.
 */
struct stream_logging {
    bool is_enabled;                          // Enable/disable stream logging
    std::string stream_log_root;              // Directory for stream log files
    std::vector<std::string> output_formats;  // Output formats: arrow_ipc, jsonl, parquet, etc.
};

/*
 * ===========================
 * Execution / Performance Controls
 * ===========================
 * Advanced tuning with 41+ parameters for fine-grained control over:
 *  - Parallelism strategy and scope (days, assets, files)
 *  - Worker thread management and scaling
 *  - Memory efficiency and resource constraints
 *  - Batch sizing for specialized engines (fill, greeks, futures, TTE)
 * 
 * Default values assume server-grade hardware with multiple cores and
 * sufficient memory. Adjust for resource-constrained environments.
 * 
 * Performance Guidelines:
 *  - CPU-bound: Increase worker counts, disable low_memory_mode
 *  - Memory-constrained: Enable low_memory_mode, reduce chunk sizes
 *  - Multi-core systems: Enable parallelism flags, increase worker caps
 *  - Single-core: Disable parallelism, use single worker threads
 */
struct execution {
    // Core I/O and Memory Settings
    int io_chunk_size = 0;                          // Chunk size for file I/O (0 = auto)
    bool low_memory_mode = false;                   // Trade performance for lower memory usage
    
    // Global Parallelism Control
    bool enable_parallelism = true;                 // Master switch for all parallel processing
    int global_worker_cap = 10;                     // Global maximum worker threads across all engines
    
    // Day-level Parallelism
    bool parallelize_days = true;                   // Process multiple trading days in parallel
    int day_worker_cap = 10;                        // Max workers for day-level parallelism
    bool batch_days_mode = true;                    // Group days into batches for processing
    int days_per_batch = 5;                         // Number of days per processing batch
    int ram_limited_day_workers = 5;                // Workers with stricter memory limits
    
    // Asset-level Parallelism
    bool parallelize_assets = false;                // Process multiple assets in parallel (typically disabled)
    int asset_worker_cap = 10;                      // Max workers for asset-level parallelism
    
    // File-level Parallelism
    int total_worker_cap = 10;                      // Total worker cap across all parallelism types
    bool parallel_file_io = true;                   // Parallel file reading operations
    int file_worker_cap = 10;                       // Max workers for file I/O operations
    
    // Specialized I/O Strategies
    bool zip_streaming_mode = false;                // Stream ZIP files instead of full extraction
    bool process_pool_csv = true;                   // Use process pool for CSV parsing
    
    // Fill Engine Configuration
    bool parallel_fill_engine = true;               // Parallelize ticker fill operations
    bool multiprocess_fill_engine = true;           // Use multiprocessing for fills
    int fill_worker_cap = 10;                       // Max workers for fill engine
    int fill_batch_size = 50;                       // Rows per batch in fill operations
    bool auto_scale_fill_workers = true;            // Auto-scale fill workers based on load
    
    // Monthly Processing
    bool parallel_monthly_engine = true;            // Parallelize monthly data aggregation
    int monthly_worker_cap = 10;                    // Max workers for monthly processing
    
    // Futures Processing
    bool parallel_futures_engine = true;            // Parallelize futures calculations
    int futures_worker_cap = 10;                    // Max workers for futures processing
    
    // Greeks Calculation (Option Greeks: Delta, Gamma, Vega, Theta, Rho)
    bool parallel_greeks_engine = true;             // Parallelize Greeks calculations
    int greeks_worker_cap = 10;                     // Max workers for Greeks calculations
    int greeks_block_size = 100000;                 // Rows per block in Greeks calculations
    
    // Ticker Transform
    int transform_worker_cap = 10;                  // Max workers for ticker transformations
    int transform_block_size = 1000;                // Rows per block in transformations
    
    // Time-to-Expiry (TTE) Calculation
    bool parallel_tte_engine = true;                // Parallelize time-to-expiry calculations
    int tte_worker_cap = 10;                        // Max workers for TTE calculations
    int tte_block_size = 500000;                    // Rows per block in TTE calculations
    
    // Synthetic Futures
    bool parallel_synthetic_futures = true;         // Parallelize synthetic futures calculations
    int syn_fut_worker_cap = 10;                    // Max workers for synthetic futures
    int syn_fut_block_size = 500000;                // Rows per block in synthetic futures
    
    // Memory Management
    bool use_memory_controller = false;             // Enable memory controller for explicit management
    bool disable_memory_controller = true;          // Disable memory controller (inverse flag)
    
    // Caching and Optimization
    bool cache_monthly_expiries = true;             // Cache computed monthly expiry sets
    bool omit_spot_iv = false;                      // Skip spot IV calculation (faster, less accurate)
    
    // Batch Scaling
    int batch_scaling_factor = 4;                   // Multiplier for dynamic batch sizing
}; 
};

/*
 * ===========================
 * Post-Compute Controls
 * ===========================
 * Optional quantitative transformations applied after base data processing.
 * These computationally intensive operations are applied selectively based on use case.
 */
struct post_compute {
    bool compute_synthetic_futures = false;  // Calculate synthetic futures contracts from options data
    bool recompute_theoretical_greeks = false; // Recalculate option Greeks using pricing models
};

/*
 * ===========================
 * Market Constants
 * ===========================
 * Market-wide constants, trading calendars, and time rules.
 * Shared across all processing stages and used for:
 *  - Time-to-expiry calculations
 *  - Trading calendar normalization
 *  - Market hours computation
 *  - Holiday accounting
 */
struct market_constants {
    // Valid Assets and Exception Handling
    std::vector<std::string> valid_underlyings;    // List of supported assets
    std::vector<std::string> symbol_exceptions;    // Assets with special naming or formatting conventions
    
    // Expiry Configuration
    std::vector<int> expiry_cutoff_time;           // Market close time for expiry [HH, MM, SS]
    
    // Month Mapping Tables
    std::map<std::string, std::string> calendar_month_map;  // Calendar months: JAN->01, FEB->02, etc.
    std::map<std::string, std::string> numeric_month_map;   // Numeric months: 1->01, 01->01, etc.
    std::map<std::string, std::string> alpha_month_map;     // Single letter months: O->10, N->11, D->12
    
    /*
     * Market Timing and Calendar
     * ==========================
     * Trading hours and session information for market time calculations.
     */
    struct marketTiming {
        std::string session_open;           // Market open time (HH:MM:SS)
        std::string session_close;          // Market close time (HH:MM:SS)
        int minutes_per_session;            // Trading minutes per session
        int sessions_per_year;              // Trading sessions per year (typically 252)
    } market_timing;

    std::vector<std::string> exchange_holidays;    // Non-trading dates in YYYY-MM-DD format
};

bool config::load_from_file(const std::string& file_name){
    // Attempt to open the configuration JSON file
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr<<"Failed to open the config file: "<< file_name << std::endl;
        return false;
    }
    
    // Parse JSON from file stream
    json j;
    try{
        file>>j;
    } catch (const json::parse_error& e){
        std::cerr<<"JSON parse error: "<<e.what()<<std::endl;
        return false;
    }
    
    // Delegate to core parser
    return parse_json(j);
};


bool config::load_from_string(const std::string& config_string){
    // Parse JSON from in-memory string
    json j;
    try {
        j = json::parse(config_string);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parsing error from config string: " <<e.what()<<std::endl;
        return false;
    }
    
    // Delegate to core parser
    return parse_json(j); 
};

bool config::parse_json(const json& j){
    try {
        // allocate and populate path config (new 'data_paths' only)
        if (j.contains("data_paths")){
            auto& p = j.at("data_paths");
            data_paths* path_config = new data_paths();
            path_config->derivatives_root = p.value("derivatives_root", std::string());
            path_config->spot_root = p.value("spot_root", std::string());
            path_config->export_root = p.value("export_root", std::string());
            path_config->log_root = p.value("log_root", path_config->export_root);
            this->data_paths_config = path_config;
        } else {
            this->data_paths_config = nullptr;
        }

        // extraction (new 'data_scope' only)
        if (j.contains("data_scope")){
            auto& e = j.at("data_scope");
            data_scope* data_scope_config = new data_scope();
            data_scope_config->underlyings = e.value("underlyings", std::vector<std::string>());
            data_scope_config->date_from = e.value("date_from", std::string());
            data_scope_config->date_to = e.value("date_to", std::string());
            data_scope_config->instrument_classes = e.value("instrument_classes", std::vector<std::string>());
            data_scope_config->expiry_limit = e.value("expiry_limit", 0);
            this->data_scope_config = data_scope_config;
        } else {
            this->data_scope_config = nullptr;
        }

        // asset_mappings (new 'symbol_registry' only)
        if (j.contains("symbol_registry")){
            auto& m = j.at("symbol_registry");
            symbol_registry* am = new symbol_registry();
            if (m.is_object()){
                for (auto it = m.begin(); it != m.end(); ++it){
                    if (it.value().is_object()){
                        am->mappings[it.key()] = it.value().get<std::map<std::string,std::string>>();
                    }
                }
            }
            this->symbol_registry_config = am;
        } else {
            this->symbol_registry_config = nullptr;
        }

        // matching (new 'symbol_matching' only)
        if (j.contains("symbol_matching")){
            auto& mm = j.at("symbol_matching");
            symbol_matching* mc = new symbol_matching();
            mc->options_mode = mm.value("options_mode", std::string());
            mc->futures_mode = mm.value("futures_mode", std::string());
            mc->index_mode = mm.value("index_mode", std::string());
            mc->is_case_sensitive = mm.value("is_case_sensitive", false);
            mc->trim_whitespace = mm.value("trim_whitespace", false);
            this->symbol_matching_config = mc;
        } else this->symbol_matching_config = nullptr;

        // processing (new 'preprocessing' only)
        if (j.contains("preprocessing")){
            auto& pr = j.at("preprocessing");
            preprocessing* pc = new preprocessing();
            pc->forward_fill = pr.value("forward_fill", false);
            pc->backward_fill = pr.value("backward_fill", false);
            pc->ignore_empty_files = pr.value("ignore_empty_files", false);
            pc->merge_daily_outputs = pr.value("merge_daily_outputs", false);
            this->preprocessing_config = pc;
        } else this->preprocessing_config = nullptr;

        // gpu (new 'acceleration' only)
        if (j.contains("acceleration")){
            auto& gr = j.at("acceleration");
            acceleration* g = new acceleration();
            g->enable_gpu = gr.value("enable_gpu", false);
            this->acceleration_config = g;
        } else this->acceleration_config = nullptr;

        // logging (new 'logger' only)
        if (j.contains("logger")){
            auto& lr = j.at("logger");
            logger* lg = new logger();
            lg->stdout_level = lr.value("stdout_level", std::string("info"));
            lg->file_log_level = lr.value("file_log_level", std::string("info"));
            lg->log_template = lr.value("log_template", std::string());
            lg->timestamp_format = lr.value("timestamp_format", std::string());
            this->logger_config = lg;
        } else this->logger_config = nullptr;

        // output (new 'export' only)
        if (j.contains("export")){
            auto& orr = j.at("export");
            export_config* o = new export_config();
            o->file_format = orr.value("file_format", std::string("parquet"));
            o->codec = orr.value("codec", std::string("none"));
            this->export_config_ptr = o;
        } else this->export_config_ptr = nullptr;

        // data_logging (new 'stream_logging' only)
        if (j.contains("stream_logging")){
            auto& dl = j.at("stream_logging");
            stream_logging* d = new stream_logging();
            d->is_enabled = dl.value("is_enabled", false);
            d->stream_log_root = dl.value("stream_log_root", std::string());
            d->output_formats = dl.value("output_formats", std::vector<std::string>());
            this->stream_logging_config = d;
        } else this->stream_logging_config = nullptr;

        // advanced (new 'execution' only)
        if (j.contains("execution")){
            auto& ar = j.at("execution");
            execution* a = new execution();
            a->io_chunk_size = ar.value("io_chunk_size", a->io_chunk_size);
            a->low_memory_mode = ar.value("low_memory_mode", a->low_memory_mode);
            a->enable_parallelism = ar.value("enable_parallelism", a->enable_parallelism);
            a->global_worker_cap = ar.value("global_worker_cap", a->global_worker_cap);
            a->parallelize_days = ar.value("parallelize_days", a->parallelize_days);
            a->day_worker_cap = ar.value("day_worker_cap", a->day_worker_cap);
            a->batch_days_mode = ar.value("batch_days_mode", a->batch_days_mode);
            a->days_per_batch = ar.value("days_per_batch", a->days_per_batch);
            a->ram_limited_day_workers = ar.value("ram_limited_day_workers", a->ram_limited_day_workers);
            a->parallelize_assets = ar.value("parallelize_assets", a->parallelize_assets);
            a->asset_worker_cap = ar.value("asset_worker_cap", a->asset_worker_cap);
            a->total_worker_cap = ar.value("total_worker_cap", a->total_worker_cap);
            a->parallel_file_io = ar.value("parallel_file_io", a->parallel_file_io);
            a->file_worker_cap = ar.value("file_worker_cap", a->file_worker_cap);
            a->zip_streaming_mode = ar.value("zip_streaming_mode", a->zip_streaming_mode);
            a->process_pool_csv = ar.value("process_pool_csv", a->process_pool_csv);
            a->parallel_fill_engine = ar.value("parallel_fill_engine", a->parallel_fill_engine);
            a->multiprocess_fill_engine = ar.value("multiprocess_fill_engine", a->multiprocess_fill_engine);
            a->fill_worker_cap = ar.value("fill_worker_cap", a->fill_worker_cap);
            a->fill_batch_size = ar.value("fill_batch_size", a->fill_batch_size);
            a->auto_scale_fill_workers = ar.value("auto_scale_fill_workers", a->auto_scale_fill_workers);
            a->parallel_monthly_engine = ar.value("parallel_monthly_engine", a->parallel_monthly_engine);
            a->monthly_worker_cap = ar.value("monthly_worker_cap", a->monthly_worker_cap);
            a->parallel_futures_engine = ar.value("parallel_futures_engine", a->parallel_futures_engine);
            a->futures_worker_cap = ar.value("futures_worker_cap", a->futures_worker_cap);
            a->parallel_greeks_engine = ar.value("parallel_greeks_engine", a->parallel_greeks_engine);
            a->greeks_worker_cap = ar.value("greeks_worker_cap", a->greeks_worker_cap);
            a->greeks_block_size = ar.value("greeks_block_size", a->greeks_block_size);
            a->transform_worker_cap = ar.value("transform_worker_cap", a->transform_worker_cap);
            a->transform_block_size = ar.value("transform_block_size", a->transform_block_size);
            a->parallel_tte_engine = ar.value("parallel_tte_engine", a->parallel_tte_engine);
            a->tte_worker_cap = ar.value("tte_worker_cap", a->tte_worker_cap);
            a->tte_block_size = ar.value("tte_block_size", a->tte_block_size);
            a->parallel_synthetic_futures = ar.value("parallel_synthetic_futures", a->parallel_synthetic_futures);
            a->syn_fut_worker_cap = ar.value("syn_fut_worker_cap", a->syn_fut_worker_cap);
            a->syn_fut_block_size = ar.value("syn_fut_block_size", a->syn_fut_block_size);
            a->use_memory_controller = ar.value("use_memory_controller", a->use_memory_controller);
            a->disable_memory_controller = ar.value("disable_memory_controller", a->disable_memory_controller);
            a->cache_monthly_expiries = ar.value("cache_monthly_expiries", a->cache_monthly_expiries);
            a->omit_spot_iv = ar.value("omit_spot_iv", a->omit_spot_iv);
            a->batch_scaling_factor = ar.value("batch_scaling_factor", a->batch_scaling_factor);
            this->execution_config = a;
        } else this->execution_config = nullptr;

        // post_processing (new 'post_compute' only)
        if (j.contains("post_compute")){
            auto& pr = j.at("post_compute");
            post_compute* pp = new post_compute();
            pp->compute_synthetic_futures = pr.value("compute_synthetic_futures", false);
            pp->recompute_theoretical_greeks = pr.value("recompute_theoretical_greeks", false);
            this->post_compute_config = pp;
        } else this->post_compute_config = nullptr;

        // constants (new 'market_constants' only)
        if (j.contains("market_constants")){
            auto& cr = j.at("market_constants");
            market_constants* c = new market_constants();
            c->valid_underlyings = cr.value("valid_underlyings", std::vector<std::string>());
            c->symbol_exceptions = cr.value("symbol_exceptions", std::vector<std::string>());
            c->expiry_cutoff_time = cr.value("expiry_cutoff_time", std::vector<int>());
            if (cr.contains("calendar_month_map") && cr.at("calendar_month_map").is_object()){
                c->calendar_month_map = cr.at("calendar_month_map").get<std::map<std::string,std::string>>();
            }
            if (cr.contains("numeric_month_map") && cr.at("numeric_month_map").is_object()){
                c->numeric_month_map = cr.at("numeric_month_map").get<std::map<std::string,std::string>>();
            }
            if (cr.contains("alpha_month_map") && cr.at("alpha_month_map").is_object()){
                c->alpha_month_map = cr.at("alpha_month_map").get<std::map<std::string,std::string>>();
            }
            if (cr.contains("trading_schedule") && cr.at("trading_schedule").is_object()){
                auto& mt = cr.at("trading_schedule");
                c->market_timing.session_open = mt.value("session_open", std::string());
                c->market_timing.session_close = mt.value("session_close", std::string());
                // minutes_per_session may be float in new config; coerce to int
                if (mt.contains("minutes_per_session")){
                    c->market_timing.minutes_per_session = static_cast<int>(mt.value("minutes_per_session", 0.0));
                } else {
                    c->market_timing.minutes_per_session = mt.value("minutes_per_day", 0);
                }
                c->market_timing.sessions_per_year = mt.value("sessions_per_year", mt.value("trading_days_per_year", 252));
            }
            c->exchange_holidays = cr.value("exchange_holidays", std::vector<std::string>());
            this->market_constants_config = c;
        } else this->market_constants_config = nullptr;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config JSON: " << e.what() << std::endl;
        return false;
    }
};

// ===========================
// Simple CLI Runner
// ===========================
// Demonstrates basic configuration loading and displays a summary of loaded settings.
// Usage: ./config_parser [config_file.json]
int main(int argc, char** argv){
    // Determine configuration file to load
    std::string cfg_file = "config.json";
    if (argc > 1) cfg_file = argv[1];

    // Instantiate and load configuration
    config cfg;
    if (!cfg.load_from_file(cfg_file)){
        std::cerr<<"Failed to load configuration from "<<cfg_file<<std::endl;
        return 1;
    }

    // Display loaded configuration summary
    std::cout<<"Configuration loaded from: "<<cfg_file<<"\n";
    
    // Display data_paths configuration
    if (cfg.data_paths_config){
        std::cout<<"Paths:\n";
        std::cout<<" - derivatives_root: "<<cfg.data_paths_config->derivatives_root<<"\n";
        std::cout<<" - spot_root: "<<cfg.data_paths_config->spot_root<<"\n";
        std::cout<<" - export_root: "<<cfg.data_paths_config->export_root<<"\n";
        std::cout<<" - log_root: "<<cfg.data_paths_config->log_root<<"\n";
    }

    // Display data_scope configuration
    if (cfg.data_scope_config){
        std::cout<<"Data Scope:\n";
        std::cout<<" - underlyings count: "<<cfg.data_scope_config->underlyings.size()<<"\n";
        std::cout<<" - date_from: "<<cfg.data_scope_config->date_from<<"\n";
        std::cout<<" - date_to: "<<cfg.data_scope_config->date_to<<"\n";
    }

    // Display symbol_registry configuration
    if (cfg.symbol_registry_config){
        std::cout<<"Symbol Registry Groups: "<<cfg.symbol_registry_config->mappings.size()<<"\n";
    }

    std::cout<<"Done."<<std::endl;
    return 0;
}