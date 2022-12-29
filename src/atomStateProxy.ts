import { v4 as uuid } from "uuid";
import { createHash } from "crypto";
import { Atom } from "./atom";
import { DeepImmutable } from "./types/common/immutable";

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
        console.log("ATOM REF", atomRef);
        const result = sharedMemoryNode.createAtom(atomRef, JSON.stringify(state));
        console.log("CREATE RESULT", result);
        console.log("PARSED", JSON.parse(result));
    }

    getState<T>(atom: Atom<T>): DeepImmutable<T> {
        const value = sharedMemoryNode.getAtomValue(atom["@@ref"]);
        console.log("asdfasdf");
        console.log("VALUE", typeof value);
        return JSON.parse(value);
    }

    setState<T>(atom: Atom<T>, updateFn: (...args: any[]) => T): void {
        sharedMemoryNode.compareAndSwap(atom["@@ref"], (loadedValue: string) => {
            const parsed = JSON.parse(loadedValue);
            updateFn(loadedValue, )
        });
    }

    deleteAtom<T>(atom: Atom<T>): void {
        sharedMemoryNode.deleteAtom(atom["@@ref"]);
    }
}
