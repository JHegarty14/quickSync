import { Atom } from "../atom";
import { deref } from "../deref";
import { reset } from "../reset";

describe("Reset Atom tests", () => {
    it("Should call the NAPI ResetAtom method and reset the atom", () => {
        const atom = Atom.from(1);
        reset(atom, 2);

        expect(deref(atom)).toEqual(2);
    });

    it("Should throw an error if validation fails", () => {
        const atom = Atom.from<number>(1, (x: number) => x > 0);

        try {
            reset(atom, -1);
        } catch (err: any) {
            expect((err as Error).message).toEqual("Invalid new state for atom");
        }
    });
});