import { workerData, parentPort } from "worker_threads";
import { Atom, swap } from "../..";

export type IWorkerData<T> = {
    atom: Atom<T>;
}

async function concurrentProcess<T>(args: IWorkerData<T>): Promise<void> {
    const { atom } = args;
    const updateFn = (num: number) => num + 2;
    swap(atom, updateFn);
    return parentPort?.postMessage(true);
}

concurrentProcess(workerData);
