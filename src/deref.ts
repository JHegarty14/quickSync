import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { throwIsNotAtom } from "./throwIsNotAtom";
import { DeepImmutable } from "./types/common/immutable.type";

export function deref<T>(atom: Atom<T>): DeepImmutable<T> {
    throwIsNotAtom(atom);
    const state = AtomStateProxy.getState(atom);
    return state;
}