import { AtomStateProxy } from "./atomStateProxy";

export class Atom<T> {
    public readonly ["@@ref"]: string;

    private constructor(state: T) {
        const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
        this["@@ref"] = AtomStateProxy._useNextAtomId();
        atomStateProxy.createAtom(this["@@ref"], state);
    }

    public static from<T>(state: T): Atom<T> {
        return new Atom(state);
    }

    public destroy(): void {
        const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
        atomStateProxy.deleteAtom(this);
    }
}
