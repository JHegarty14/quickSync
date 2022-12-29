import { Atom } from ".";
import { AtomStateProxy } from "./atomStateProxy";

export function swapWorker<T>(filepath: string, data: Atom<T>) {
    const atomStateInstance = AtomStateProxy.getAtomStateProxyInstance();
    return new Promise((resolve, reject) => {
        const worker = new Worker(filepath);
        const sharedAtomState = new SharedArrayBuffer(Int32Array.BYTES_PER_ELEMENT)
    });
}