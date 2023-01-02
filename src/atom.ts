import { AtomStateProxy } from "./atomStateProxy";
import { Nullable } from "./types/common/nullable.type";
import { ValidatorFn } from "./types/domain/validatorFn.type";

export class Atom<T> {
    public readonly ["@@ref"]: string;

    public validatorFn: Nullable<ValidatorFn<T>>;

    private constructor(state: T, validatorFn?: ValidatorFn<T>) {
        const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
        this["@@ref"] = AtomStateProxy._useNextAtomId();
        atomStateProxy.createAtom(this["@@ref"], state);
        this.validatorFn = validatorFn ?? null;
    }

    public static from<T>(state: T, validatorFn?: ValidatorFn<T>): Atom<T> {
        return new Atom(state, validatorFn);
    }

    public setValidator(validatorFn: ValidatorFn<T>): void {
        this.validatorFn = validatorFn;
    }
}
