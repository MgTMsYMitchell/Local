# Diagrams Index

Quick-reference index of all architecture diagrams. GitHub renders Mermaid
natively in markdown files; click any link to view the diagram and full
component notes.

## Architecture diagrams

| # | Diagram | Description |
|---|---|---|
| 1 | [System Architecture Overview](../architecture/01-system-overview.md) | End-to-end stack: clients → agent framework → subsystems → storage |
| 2 | [Agent Framework](../architecture/02-agent-framework.md) | Agent manager, task graph, tool registry, memory store, execution loop |
| 3 | [RQr2 Module](../architecture/03-rqr2-module.md) | 14D symbol encoding pipeline: quantize → reduce → reconstruct |
| 4 | [LLM Integration Layer](../architecture/04-llm-integration.md) | LLM router, local/remote clients, embedding engine, prompt templates |
| 5 | [Self-Healing Engine](../architecture/05-self-healing.md) | Health monitor, anomaly detector, recovery strategies, rollback |
| 6 | [Mesh Networking Layer](../architecture/06-mesh-networking.md) | Gossip membership, adaptive routing, NAT traversal, QoS |
| 7 | [Storage Layer](../architecture/07-storage.md) | SQLite WAL, symbol store, knowledge substrate, audit log |
| 8 | [Compute Pipeline](../architecture/08-compute-pipeline.md) | CPU/GPU scheduling, SIMD kernels, operator fusion |
| 9 | [CI/CD Pipeline](../architecture/09-cicd-pipeline.md) | Matrix build, sanitizers, coverage, container publishing |

## Mermaid quick-start

All diagrams use [Mermaid](https://mermaid.js.org/) flowchart syntax and render
directly on GitHub. To generate static image exports locally:

```bash
# requires Node.js + @mermaid-js/mermaid-cli
npx mmdc -i docs/architecture/01-system-overview.md -o docs/diagrams/01-system-overview.svg
```

Or use the VS Code **Mermaid Preview** extension for an in-editor live preview.
