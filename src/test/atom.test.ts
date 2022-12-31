import { Atom } from "..";

describe("Atom instance", () => {
  it("can be instantiated by the static Atom.of method", () => {
    expect(Atom.from(1)).toBeInstanceOf(Atom);
  });

  it("has a readonly public instance member @@ref which is a number that acts as its unique ID", () => {
    const a = Atom.from(1);
    expect(a instanceof Atom<number>).toBeTruthy();

    expect(() => {
      (a as any).uwrap()["@@ref"] = 1000e10;
    }).toThrow(TypeError);
  });
});