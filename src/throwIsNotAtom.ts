import { Result, err, ok } from "./lib/result";

export function throwIsNotAtom(atom: any): Result<
    boolean,
    string
> {
    if (!atom["@@ref"]) {
        const atomStr = atom.toString();
        console.log("ATOM STR", atomStr);
        return err(`Provided value \n${atomStr}\n is not a valid atom.`);
    }

    return ok(true);
}
