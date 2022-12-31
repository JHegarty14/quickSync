import { Atom } from "../atom";
import { deref } from "../deref";
import { destroy } from "../destroy";

describe("Destroy Atom tests", () => {
    it("Should call the NAPI DeleteAtom method and destroy the atom", () => {
        const atom = Atom.from(1);
        destroy(atom);

        try {
            deref(atom);
        } catch (err: any) {
            expect((err as Error).message).toEqual("Atom not found");
        }
    });

    it("Shoud throw an error if the atom is already destroyed", () => {
        const atom = Atom.from(1);
        destroy(atom);

        try {
            destroy(atom);
        } catch (err: any) {
            expect((err as Error).message).toEqual("Atom not found")
        }
    });
});