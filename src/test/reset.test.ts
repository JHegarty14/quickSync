import { Atom } from "../atom";
import { deref } from "../deref";
import { reset } from "../reset";

describe("Reset Atom tests", () => {
    it("Should call the NAPI ResetAtom method and reset the atom", () => {
        const atom = Atom.from(1);
        reset(atom, 2);

        expect(deref(atom)).toEqual(2);
    });
});