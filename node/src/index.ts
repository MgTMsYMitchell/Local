import { RslVM, Instr } from './vm';

const vm = new RslVM();

export function runTaskLog(program: Instr[], refs: string[]) {
  return vm.executeTaskLog(program, { refs });
}

if (require.main === module) {
  console.log('RN2 node runtime stub. Wire HTTP/WebSocket API here.');
}
