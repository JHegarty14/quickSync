import { v4 as uuid } from "uuid";
import { createHash } from "crypto";
import { Atom } from "./atom";
import { DeepImmutable } from "./types/common/immutable";
import { SwapFn } from "./types/domain/swap";

const sharedMemoryNode = require("bindings")("sharedMemoryNode.node");

export class AtomStateProxy {
    private static instance: AtomStateProxy;

    private constructor() {}

    static getAtomStateProxyInstance(): AtomStateProxy {
        if (!AtomStateProxy.instance) {
            AtomStateProxy.instance = new AtomStateProxy();
        }

        return AtomStateProxy.instance;
    }

    static _useNextAtomId(): string {
        const md5 = createHash("md5").update(uuid()).digest("hex");
        return md5.substring(0, 30);
    }

    createAtom<T>(atomRef: string, state: T): void {
        const payload = { state };
        sharedMemoryNode.createAtom(atomRef, JSON.stringify(payload));
    }

    getState<T>(atom: Atom<T>): DeepImmutable<T> {
        const value = sharedMemoryNode.getAtomValue(atom["@@ref"]);
        return JSON.parse(value).state;
    }

    setState<T>(atom: Atom<T>, updateFn: SwapFn<T>, ...args: any[]): void {
        sharedMemoryNode.compareAndSwap(atom["@@ref"], (loadedValue: string) => {
            const parsed = JSON.parse(loadedValue).state;
            const state = updateFn(parsed, ...args);
            if (atom.validatorFn) {
                const valid = atom.validatorFn(state);
                if (!valid) throw new Error("Invalid new state for atom");
            }
            return JSON.stringify({ state });
        });
    }

    deleteAtom<T>(atom: Atom<T>): void {
        sharedMemoryNode.deleteAtom(atom["@@ref"]);
    }

    resetAtom<T>(atom: Atom<T>, value: T): void {
        const payload = { state: value };
        if (atom.validatorFn) {
            const valid = atom.validatorFn(value);
            if (!valid) throw new Error("Invalid new state for atom");
        }
        sharedMemoryNode.resetAtom(atom["@@ref"], JSON.stringify(payload));
    }
}
