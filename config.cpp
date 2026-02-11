/*
 * ===========================
 * Standard & Third-Party Includes
 * ===========================
 * iostream      : basic IO utilities
 * string        : std::string support
 * fstream       : file read/write for config loading
 * vector/map    : STL containers used across config objects
 * nlohmann/json : JSON parsing and object mapping
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
 * Each pointer corresponds to a logically isolated config module.
 *
 * Design choice:
 *  - Pointers allow optional sections and late initialization
 *  - JSON parsing is centralized and delegated internally
 */
class config {
public:
    data_paths* data_paths_config;
    data_scope* data_scope_config;
    symbol_registry* symbol_registry_config;
    symbol_matching* symbol_matching_config;
    preprocessing* preprocessing_config;
    acceleration* acceleration_config;        
    logger* logger_config;
    export_config* export_config_ptr;
    stream_logging* stream_logging_config;
    execution* execution_config;
    post_compute* post_compute_config;
    market_constants* market_constants_config;

    config() = default;

    /*
     * Load configuration from a JSON file on disk.
     * Returns false if:
     *  - file cannot be opened
     *  - JSON parsing fails
     *  - required fields are missing
     */
    bool load_from_file(const std::string& file_name);

    /*
     * Load configuration from an in-memory JSON string.
     * Useful for:
     *  - testing
     *  - remote config injection
     *  - dynamic runtime overrides
     */
    bool load_from_string(const std::string& config_str);

    std::map<std::string, std::string> to_flat_map() const;

private:
    /*
     * Core JSON parser.
     * Maps validated JSON fields into strongly typed config structs.
     */
    bool parse_json(const json& j);
};



struct data_paths {
    std::string derivatives_root;
    std::string spot_root;
    std::string export_root;
    std::string log_root;
};
/*
 * ===========================
 * Data Scope Configuration
 * ===========================
 * Defines raw data input paths and output/log directories.
 * Typically used by data ingestion pipelines.
 */
struct data_scope {
    std::vector<std::string> underlyings;
    std::string date_from;
    std::string date_to;
    std::vector<std::string> instrument_classes;
    int expiry_limit = 0;  // Default to 0 as per comment
};

/*
 * ===========================
 * Symbol Registry Configuration
 * ===========================
 * Maps logical asset identifiers to exchange-specific symbols.
 * Nested maps allow multi-exchange or multi-instrument mapping.
 */
struct symbol_registry {
    std::map<std::string, std::map<std::string, std::string>> mappings;
};

/*
 * ===========================
 * Symbol Matching Rules
 * ===========================
 * Controls how futures/options/index instruments are matched.
 * Affects symbol normalization and lookup behavior.
 */
struct symbol_matching {
    std::string options_mode;
    std::string futures_mode;
    std::string index_mode;
    bool is_case_sensitive;
    bool trim_whitespace;
};

/*
 * ===========================
 * Preprocessing Rules
 * ===========================
 * Defines data-cleaning and aggregation behaviors.
 */
struct preprocessing {
    bool backward_fill;         // backward fill missing values
    bool forward_fill;          // forward fill missing values
    bool ignore_empty_files;    // ignore zero-row datasets
    bool merge_daily_outputs;   // merge daily partitions
};

/*
 * ===========================
 * Acceleration Configuration
 * ===========================
 * Toggles GPU kernel acceleration where available.
 */
struct acceleration {
    bool enable_gpu;
};

/*
 * ===========================
 * Logger Configuration
 * ===========================
 * Controls verbosity, formatting, and timestamp styles.
 */
struct logger {
    std::string stdout_level;
    std::string file_log_level;
    std::string log_template;
    std::string timestamp_format;
};

/*
 * ===========================
 * Export Configuration
 * ===========================
 * Defines final data serialization format and compression.
 */
struct export_config {
    std::string file_format;
    std::string codec;
};

/*
 * ===========================
 * Stream Logging Configuration
 * ===========================
 * Optional fine-grained data snapshots for debugging/audit.
 */
struct stream_logging {
    bool is_enabled;
    std::string stream_log_root;
    std::vector<std::string> output_formats;
};

/*
 * ===========================
 * Execution / Performance Controls
 * ===========================
 * High-granularity control over:
 *  - parallelism
 *  - batching
 *  - memory usage
 *  - worker scaling
 *
 * Defaults are aggressive and assume server-grade hardware.
 */
struct execution {
    int io_chunk_size = 0;  // 0 treated as null / auto
    bool low_memory_mode = false;
    bool enable_parallelism = true;
    int global_worker_cap = 10;
    bool parallelize_days = true;
    int day_worker_cap = 10;
    bool batch_days_mode = true;
    int days_per_batch = 5;
    int ram_limited_day_workers = 5;
    bool parallelize_assets = false;
    int asset_worker_cap = 10;
    int total_worker_cap = 10;
    bool parallel_file_io = true;
    int file_worker_cap = 10;
    bool zip_streaming_mode = false;
    bool process_pool_csv = true;
    bool parallel_fill_engine = true;
    bool multiprocess_fill_engine = true;
    int fill_worker_cap = 10;
    int fill_batch_size = 50;
    bool auto_scale_fill_workers = true;
    bool parallel_monthly_engine = true;
    int monthly_worker_cap = 10;
    bool parallel_futures_engine = true;
    int futures_worker_cap = 10;
    bool parallel_greeks_engine = true;
    int greeks_worker_cap = 10;
    int greeks_block_size = 100000;
    int transform_worker_cap = 10;
    int transform_block_size = 1000;
    bool parallel_tte_engine = true;
    int tte_worker_cap = 10;
    int tte_block_size = 500000;
    bool parallel_synthetic_futures = true;
    int syn_fut_worker_cap = 10;
    int syn_fut_block_size = 500000;
    bool use_memory_controller = false;
    bool disable_memory_controller = true;
    bool cache_monthly_expiry_set = true;
    bool omit_spot_iv = false;
    int batch_scaling_factor = 4; 
};

/*
 * ===========================
 * Post-Compute Controls
 * ===========================
 * Optional quantitative transformations applied after base processing.
 */
struct post_compute {
    bool compute_synthetic_futures = false;
    bool recompute_theoretical_greeks = false;
};

/*
 * ===========================
 * Market Constants
 * ===========================
 * Market-wide constants, calendars, and time rules.
 * Shared across all processing stages.
 */
struct market_constants {
    std::vector<std::string> valid_underlyings;
    std::vector<std::string> symbol_exceptions;
    std::vector<int> expiry_cutoff_time;
    std::map<std::string, std::string> calendar_month_map;
    std::map<std::string, std::string> numeric_month_map;
    std::map<std::string, std::string> alpha_month_map;

    /*
     * Market timing parameters used in time-to-expiry,
     * trading calendar normalization, and simulations.
     */
    struct marketTiming {
        std::string session_open;
        std::string session_close;
        int minutes_per_session;
        int sessions_per_year;
    } market_timing;

    std::vector<std::string> exchange_holidays;
};

bool config::load_from_file(const std::string& file_name){
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr<<"Failed to open the config file: "<< file_name << std::endl;
        return false;
    };
    json j;
    try{
        file>>j;
    } catch (const json::parse_error& e){
        std::cerr<<"Json parse text: "<<e.what()<<std::endl;
        return false;
    }
    return parse_json(j);
};


bool config::load_from_string(const std::string& config_string){
    json j;
    try {
        j = json::parse(config_string);
    } catch (const json::parse_error& e) {
        std::cerr << "Json paring error coming from 'config.json': " <<e.what()<<std::endl;
        return false;
    }
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
            a->cache_monthly_expiry_set = ar.value("cache_monthly_expiry_set", a->cache_monthly_expiry_set);
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

// Simple CLI runner to load and display a compact summary of the config
int main(int argc, char** argv){
    std::string cfg_file = "config.json";
    if (argc > 1) cfg_file = argv[1];

    config cfg;
    if (!cfg.load_from_file(cfg_file)){
        std::cerr<<"Failed to load configuration from "<<cfg_file<<std::endl;
        return 1;
    }

    std::cout<<"Configuration loaded from: "<<cfg_file<<"\n";
    if (cfg.data_paths_config){
        std::cout<<"Paths:\n";
        std::cout<<" - derivatives_root: "<<cfg.data_paths_config->derivatives_root<<"\n";
        std::cout<<" - spot_root: "<<cfg.data_paths_config->spot_root<<"\n";
        std::cout<<" - export_root: "<<cfg.data_paths_config->export_root<<"\n";
        std::cout<<" - log_root: "<<cfg.data_paths_config->log_root<<"\n";
    }

    if (cfg.data_scope_config){
        std::cout<<"Extraction:\n";
        std::cout<<" - underlyings count: "<<cfg.data_scope_config->underlyings.size()<<"\n";
        std::cout<<" - date_from: "<<cfg.data_scope_config->date_from<<"\n";
        std::cout<<" - date_to: "<<cfg.data_scope_config->date_to<<"\n";
    }

    if (cfg.symbol_registry_config){
        std::cout<<"Asset mappings groups: "<<cfg.symbol_registry_config->mappings.size()<<"\n";
    }

    std::cout<<"Done."<<std::endl;
    return 0;
}