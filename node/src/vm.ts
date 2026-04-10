export type Instr = {
  opcode: number;
  role: number;
  skillIdx: number;
  attrPacked: number;
  refIdx: number;
};

export interface VMContext {
  refs: string[];
}

export class RslVM {
  executeTaskLog(program: Instr[], ctx: VMContext): any {
    const state: any = { entries: [] };
    let current: any = null;

    for (const instr of program) {
      switch (instr.opcode) {
        case 1:
          current = { refs: [] };
          break;
        case 2:
          if (current) current.refs.push(ctx.refs[instr.refIdx] ?? null);
          break;
        case 3:
          if (current) state.entries.push(current);
          current = null;
          break;
      }
    }
    return state;
  }
}
