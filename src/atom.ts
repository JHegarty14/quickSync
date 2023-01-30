import { AtomStateProxy } from "./atomStateProxy";
import { ValidatorFn } from "./types/domain/validatorFn.type";

export class Atom<T> {
    private constructor(state: T, validatorFn?: ValidatorFn<T>) {
        this["@@ref"] = AtomStateProxy._useNextAtomId();
        AtomStateProxy.createAtom(this["@@ref"], state);
        if (validatorFn) {
            AtomStateProxy.setValidator(this, validatorFn);
        }
    }

    public readonly ["@@ref"]: string;

    public static from<T>(state: T, validatorFn?: ValidatorFn<T>): Atom<T> {
        return new Atom(state, validatorFn);
    }
}
