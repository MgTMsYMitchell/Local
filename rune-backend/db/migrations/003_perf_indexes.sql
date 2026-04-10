-- Migration 003 — performance indexes
-- Added to eliminate full-table scans on FK join columns and
-- to support efficient agent-log filtering by agent name and timestamp.

-- stories → runes lookup (e.g. "find all stories for rune X")
CREATE INDEX IF NOT EXISTS idx_stories_rune_id
    ON stories(rune_id);

-- mythic_moments → runes / stories lookups
CREATE INDEX IF NOT EXISTS idx_mythic_rune_id
    ON mythic_moments(rune_id);

CREATE INDEX IF NOT EXISTS idx_mythic_story_id
    ON mythic_moments(story_id);

-- agent_log: filter by agent name and time-range queries
CREATE INDEX IF NOT EXISTS idx_agentlog_agent
    ON agent_log(agent);

CREATE INDEX IF NOT EXISTS idx_agentlog_created_at
    ON agent_log(created_at);
