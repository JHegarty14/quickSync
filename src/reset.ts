import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { throwIsNotAtom } from "./throwIsNotAtom";

export function reset<T>(atom: Atom<T>, value: T): void {
    throwIsNotAtom(atom);
    const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
    atomStateProxy.resetAtom(atom, value);
}