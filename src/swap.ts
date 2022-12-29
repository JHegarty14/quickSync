import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { prettyPrint } from "./lib/prettyPrint/prettyPrint";
import { err, ok, Result } from "./lib/result";
import { throwIsNotAtom } from "./throwIsNotAtom";
import { SwapFn } from "./types/domain/swap";

export const swap = <T>(
    atom: Atom<T>,
    updateFn: SwapFn<T>
): Result<null, string> => {
    const atomStateProxy = AtomStateProxy.getAtomStateProxyInstance();
    try {
        atomStateProxy.setState(atom, updateFn);
    } catch (e) {
        return err((e as Error).message);
    }

    return ok(null);
}