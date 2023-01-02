import { Atom } from "./atom";
import { Result, err, ok } from "./lib/result";

export function throwIsNotAtom(atom: Atom<any>): Result<
    boolean,
    string
> {
    if (!(atom instanceof Atom)) {
        return err(`Value is not a valid atom.`);
    }

    return ok(true);
}
