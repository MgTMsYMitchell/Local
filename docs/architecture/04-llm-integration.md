# LLM Integration Layer

The **LLM Router** is the single dispatch point for all inference requests.
It can target a **local model** (via `RUNE_LLM_URL` / `RUNE_LLM_MODEL`) or a
**remote API** client, and maintains a vector cache for embedding reuse.

```mermaid
flowchart TD
    A[LLM Router] --> B[Local Model Client]
    A --> C[Remote API Client]
    A --> D[Embedding Engine]

    B --> E[Tokenizer]
    C --> F[HTTP Transport]
    D --> G[Vector Cache (SQLite)]

    A --> H[Prompt Template Registry]
    A --> I[Batching Engine]
    A --> J[Streaming Token Interface]
```

## Configuration

| Env variable | Default | Purpose |
|---|---|---|
| `RUNE_LLM_URL` | *(unset)* | HTTP endpoint of the local/remote LLM (OpenAI-compat) |
| `RUNE_LLM_MODEL` | `local-model` | Model name forwarded in the request body |

When `RUNE_LLM_URL` is unset, `ChatAgent` falls back to an echo reply so the
rest of the pipeline remains functional.

## Request flow

1. Client POSTs a message to the brain via REST or enqueues a `chat_request` event.
2. `WorkerAgent` routes it as a `chat_route` event.
3. `ChatAgent` polls `chat_route` events, builds an OpenAI-compatible JSON body,
   POSTs to `RUNE_LLM_URL`, and emits a `chat_response` event.
4. The response is written to `audit_log` for heatmap analysis.

## Expected API shape

```json
POST /v1/chat/completions
{
  "model": "local-model",
  "messages": [{"role": "user", "content": "..."}],
  "stream": false
}
```

Response must contain `choices[0].message.content`.

## Related diagrams

- [Agent Framework](02-agent-framework.md)
- [System Overview](01-system-overview.md)
