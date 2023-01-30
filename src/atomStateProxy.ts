import { v4 as uuid } from "uuid";
import { createHash } from "crypto";
import { Atom } from "./atom";
import { DeepImmutable } from "./types/common/immutable.type";
import { SwapFn } from "./types/domain/swap.type";
import { ValidatorFn, WatchFn } from "./types";

const sharedMemoryNode = require("bindings")("sharedMemoryNode.node");

export class AtomStateProxy {
    private constructor() {}

    static _useNextAtomId(): string {
        const md5 = createHash("md5").update(uuid()).digest("hex");
        return md5.substring(0, 30);
    }

    static createAtom<T>(atomRef: string, state: T): void {
        const payload = { state };
        sharedMemoryNode.createAtom(atomRef, JSON.stringify(payload));
    }

    static getState<T>(atom: Atom<T>): DeepImmutable<T> {
        const value = sharedMemoryNode.getAtomValue(atom["@@ref"]);
        return JSON.parse(value).state;
    }

    static setState<T>(atom: Atom<T>, updateFn: SwapFn<T>, ...args: any[]): void {
        sharedMemoryNode.compareAndSwap(atom["@@ref"], (loadedValue: string) => {
            const parsed = JSON.parse(loadedValue).state;
            const state = updateFn(parsed, ...args);
            return JSON.stringify({ state });
        });
    }

    static deleteAtom<T>(atom: Atom<T>): void {
        sharedMemoryNode.deleteAtom(atom["@@ref"]);
    }

    static resetAtom<T>(atom: Atom<T>, value: T): void {
        const payload = { state: value };
        sharedMemoryNode.resetAtom(atom["@@ref"], JSON.stringify(payload));
    }

    static setValidator<T>(atom: Atom<T>, validatorFn: ValidatorFn<T>): void {
        sharedMemoryNode.setAtomValidator(atom["@@ref"], validatorFn);
    }

    static removeValidator<T>(atom: Atom<T>): void {
        sharedMemoryNode.removeAtomValidator(atom["@@ref"]);
    }

    static addWatcher<T>(atom: Atom<T>, watcherKey: string, fn: WatchFn<T>): void {
        sharedMemoryNode.addAtomWatcher(atom["@@ref"], watcherKey, fn);
    }

    static removeWatcher<T>(atom: Atom<T>, watcherKey: string): void {
        sharedMemoryNode.removeAtomWatcher(atom["@@ref"], watcherKey);
    }
}
