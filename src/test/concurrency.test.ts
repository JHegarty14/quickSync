import { Worker } from "worker_threads";
import { addAtomWatcher, deref, setAtomValidator } from "..";
import { Atom } from "../atom";

const concurrentWorker = <T>(atom: Atom<T>) => {
    return new Promise((resolve, reject) => {
        const worker = new Worker(
            "./src/test/scripts/migration.js",
            { 
                workerData: {
                    path: "./concurrentProcess.ts",
                    atom,
                },
            }
        );
        worker.on("message", (msg) => {
            if (msg) resolve(msg);
            else reject(msg);

            return msg;
        });
        worker.on("error", (error) => {
            console.error(error);
            worker.terminate();
        });
        worker.on("exit", (code) => {
            if (code !== 0) {
                worker.terminate();
                reject();
            }
        });
    });
};

describe("Tests concurrency safety with multithreaded processes", () => {
    jest.setTimeout(10000);

    it("Handles updates to single atom from multiple threads", async () => {
        const initValue = 1;
        const testAtom = Atom.from(initValue);
        const workers = [
            concurrentWorker(testAtom),
            concurrentWorker(testAtom),
            concurrentWorker(testAtom),
        ];
        await Promise.all(workers);

        const updatedValue = deref(testAtom);
        expect(updatedValue).toEqual(7);
    });

    it("Handles updates to multiple atoms from multiple threads and calls correct watcher", async () => {
        const atomA = Atom.from(1);
        const atomB = Atom.from(5);
        const spyA = jest.fn().mockImplementation();
        const spyB = jest.fn().mockImplementation();
        setAtomValidator(atomA, v => true);
        addAtomWatcher( atomA, "atomA", spyA);
        addAtomWatcher(atomB, "atomB", spyB);

        const workers = [
            concurrentWorker(atomA),
            concurrentWorker(atomA),
            concurrentWorker(atomB),
        ];
        await Promise.all(workers);

        const updatedValueA = deref(atomA);
        expect(updatedValueA).toEqual(5);
        const updatedValueB = deref(atomB);
        expect(updatedValueB).toEqual(7);

        await new Promise(resolve => setTimeout(resolve, 2000));

        expect(spyA).toHaveBeenCalledTimes(2);
        expect(spyB).toHaveBeenCalledTimes(1);
    });
});