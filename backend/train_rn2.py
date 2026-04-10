"""
train_rn2.py — Training loop for the RN2 tiny transformer model.

Generates synthetic RSL instruction sequences and trains the model to
predict next opcode, role, and skill at each position.
"""

import os
import random
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader

from rn2_model import RN2TinyModel

# ---------------------------------------------------------------------------
# Hyper-parameters
# ---------------------------------------------------------------------------
NUM_OPCODES = 256
NUM_ROLES = 64
NUM_SKILLS = 256
ATTR_BITS = 16
SEQ_LEN = 32
BATCH_SIZE = 32
EPOCHS = 10
LR = 1e-3
MODELS_DIR = os.path.join(os.path.dirname(__file__), "models")


# ---------------------------------------------------------------------------
# Synthetic dataset
# ---------------------------------------------------------------------------
class SyntheticRSLDataset(Dataset):
    """Generates random RSL instruction sequences for pre-training."""

    def __init__(self, size: int = 2048, seq_len: int = SEQ_LEN):
        self.size = size
        self.seq_len = seq_len

    def __len__(self):
        return self.size

    def __getitem__(self, _idx):
        op_ids = torch.randint(0, NUM_OPCODES, (self.seq_len,))
        role_ids = torch.randint(0, NUM_ROLES, (self.seq_len,))
        skill_ids = torch.randint(0, NUM_SKILLS, (self.seq_len,))
        attr_bits = torch.rand(self.seq_len, ATTR_BITS)
        dewey_ids = torch.randint(0, 1024, (1,)).expand(self.seq_len).contiguous()

        # Targets: next-token prediction (shift by 1, pad last position with 0)
        op_target = torch.cat([op_ids[1:], torch.zeros(1, dtype=torch.long)])
        role_target = torch.cat([role_ids[1:], torch.zeros(1, dtype=torch.long)])
        skill_target = torch.cat([skill_ids[1:], torch.zeros(1, dtype=torch.long)])

        return {
            "op_ids": op_ids,
            "role_ids": role_ids,
            "skill_ids": skill_ids,
            "attr_bits": attr_bits,
            "dewey_ids": dewey_ids,
            "op_target": op_target,
            "role_target": role_target,
            "skill_target": skill_target,
        }


# ---------------------------------------------------------------------------
# Training
# ---------------------------------------------------------------------------
def train():
    os.makedirs(MODELS_DIR, exist_ok=True)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Training on {device}")

    model = RN2TinyModel(
        num_opcodes=NUM_OPCODES,
        num_roles=NUM_ROLES,
        num_skills=NUM_SKILLS,
        attr_bits=ATTR_BITS,
        max_len=SEQ_LEN,
    ).to(device)

    dataset = SyntheticRSLDataset()
    loader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True)

    optimizer = torch.optim.Adam(model.parameters(), lr=LR)
    criterion = nn.CrossEntropyLoss()

    for epoch in range(1, EPOCHS + 1):
        model.train()
        total_loss = 0.0
        for batch in loader:
            op_ids = batch["op_ids"].to(device)
            role_ids = batch["role_ids"].to(device)
            skill_ids = batch["skill_ids"].to(device)
            attr_bits = batch["attr_bits"].to(device)
            dewey_ids = batch["dewey_ids"].to(device)

            op_target = batch["op_target"].to(device)
            role_target = batch["role_target"].to(device)
            skill_target = batch["skill_target"].to(device)

            optimizer.zero_grad()
            outputs = model(op_ids, role_ids, skill_ids, attr_bits, dewey_ids)

            B, T, _ = outputs["op_logits"].shape
            loss = (
                criterion(outputs["op_logits"].view(B * T, -1), op_target.view(-1))
                + criterion(outputs["role_logits"].view(B * T, -1), role_target.view(-1))
                + criterion(outputs["skill_logits"].view(B * T, -1), skill_target.view(-1))
            )
            loss.backward()
            optimizer.step()
            total_loss += loss.item()

        avg = total_loss / len(loader)
        print(f"Epoch {epoch:3d}/{EPOCHS}  loss={avg:.4f}")

    save_path = os.path.join(MODELS_DIR, "rn2_tiny.pt")
    torch.save(model.state_dict(), save_path)
    print(f"Model saved to {save_path}")


if __name__ == "__main__":
    random.seed(42)
    torch.manual_seed(42)
    train()
