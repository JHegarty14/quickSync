import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { throwIsNotAtom } from "./throwIsNotAtom";

export function destroy<T>(atom: Atom<T>): void {
    throwIsNotAtom(atom);
    const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
    atomStateProxy.deleteAtom(atom); 
}
