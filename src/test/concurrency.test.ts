import { Worker } from "worker_threads";
import { deref } from "..";
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
});