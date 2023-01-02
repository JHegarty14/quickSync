import { Atom } from "../atom";
import { throwIsNotAtom } from "../throwIsNotAtom";

describe("Test throwIsNotAtom", () => {
    it("Should not throw if the atom is valid", () => {
        const atom = Atom.from(1);
        const result = throwIsNotAtom(atom);
        expect(result.isOk()).toBeTruthy();
    });

    it("Should throw if the atom is invalid", () => {
        const atom = { "@@ref": 1 };
        const result = throwIsNotAtom(atom as any);
        expect(result.isErr()).toBeTruthy();
    });
})