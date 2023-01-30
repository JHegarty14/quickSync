import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { throwIsNotAtom } from "./throwIsNotAtom";
import { ValidatorFn } from "./types";

export function setAtomValidator<T>(atom: Atom<T>, validator: ValidatorFn<T>): void {
    throwIsNotAtom(atom);
    AtomStateProxy.setValidator(atom, validator);
}

export function removeAtomValidator<T>(atom: Atom<T>): void {
    throwIsNotAtom(atom);
    AtomStateProxy.removeValidator(atom);
}