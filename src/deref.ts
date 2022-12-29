import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { throwIsNotAtom } from "./throwIsNotAtom";
import { DeepImmutable } from "./types/common/immutable";

export function deref<T>(atom: Atom<T>): DeepImmutable<T> {
    throwIsNotAtom(atom);
    const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
    const state = atomStateProxy.getState(atom);
    return state;
}