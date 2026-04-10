import torch
import torch.nn as nn


class RN2TinyModel(nn.Module):
    def __init__(
        self,
        num_opcodes: int = 256,
        num_roles: int = 64,
        num_skills: int = 256,
        attr_bits: int = 16,
        dewey_dim: int = 32,
        d_model: int = 128,
        n_heads: int = 4,
        n_layers: int = 4,
        max_len: int = 128,
    ):
        super().__init__()
        self.op_embed = nn.Embedding(num_opcodes, d_model)
        self.role_embed = nn.Embedding(num_roles, d_model)
        self.skill_embed = nn.Embedding(num_skills, d_model)
        self.attr_proj = nn.Linear(attr_bits, d_model)
        self.dewey_embed = nn.Embedding(1024, dewey_dim)

        self.dewey_proj = nn.Linear(dewey_dim, d_model)
        self.pos_embed = nn.Embedding(max_len, d_model)
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model,
            nhead=n_heads,
            dim_feedforward=256,
            batch_first=True,
        )
        self.encoder = nn.TransformerEncoder(encoder_layer, num_layers=n_layers)

        self.op_head = nn.Linear(d_model, num_opcodes)
        self.role_head = nn.Linear(d_model, num_roles)
        self.skill_head = nn.Linear(d_model, num_skills)

    def forward(self, op_ids, role_ids, skill_ids, attr_bits, dewey_ids):
        B, T = op_ids.size()
        pos = torch.arange(T, device=op_ids.device).unsqueeze(0).expand(B, T)

        op_e = self.op_embed(op_ids)
        role_e = self.role_embed(role_ids)
        skill_e = self.skill_embed(skill_ids)
        attr_e = self.attr_proj(attr_bits)
        pos_e = self.pos_embed(pos)

        dewey_e = self.dewey_proj(self.dewey_embed(dewey_ids))
        dewey_e = dewey_e.unsqueeze(1).expand(B, T, dewey_e.size(-1))

        x = op_e + role_e + skill_e + attr_e + pos_e + dewey_e

        h = self.encoder(x)
        return {
            "op_logits": self.op_head(h),
            "role_logits": self.role_head(h),
            "skill_logits": self.skill_head(h),
        }
