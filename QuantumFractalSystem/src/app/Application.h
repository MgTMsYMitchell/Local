#pragma once
// QuantumFractalSystem — Application orchestrator

#include "compression/QuantumInspiredCompressor.h"
#include "engine/ComputeEngine.h"
#include "engine/SelfHealingEngine.h"
#include "network/FractalMesh.h"
#include "storage/Database.h"

#include <memory>
#include <string>

namespace qfs {

/// Top-level application class that wires together all subsystems:
/// database, compression, mesh network, compute engine, and self-healing.
class Application {
public:
    Application();
    ~Application();

    /// Initialise all subsystems (database, mesh, engines).
    void init(const std::string& config_path = "config/app_config.json");

    /// Run the main event loop.
    void run();

    /// Graceful shutdown.
    void shutdown();

    // Accessors
    Database&                   db()        { return *db_; }
    FractalMesh&                mesh()      { return mesh_; }
    QuantumInspiredCompressor&  compressor(){ return compressor_; }
    SelfHealingEngine&          healer()    { return healer_; }
    ComputeEngine&              compute()   { return compute_; }

private:
    std::unique_ptr<Database>   db_;
    FractalMesh                 mesh_;
    QuantumInspiredCompressor   compressor_{32};
    SelfHealingEngine           healer_{0.3f, 0.05f};
    ComputeEngine               compute_;
    bool                        running_ = false;

    void load_config(const std::string& path);
    void apply_schema();
    void seed_nodes(int count);
};

} // namespace qfs
