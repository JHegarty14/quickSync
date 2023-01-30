import { Atom, addAtomWatcher, swap } from "..";
import { AtomStateProxy } from "../atomStateProxy";
import { setAtomValidator } from "../validator";

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

  it("can accept a validator function on construction", () => {
    const a = Atom.from(1, (v) => {
      return v > 0;
    });

    expect(a instanceof Atom<number>).toBeTruthy();
    expect(AtomStateProxy.setValidator).toHaveBeenCalledWith(a, expect.any(Function));
  });

  it("can set a validator function after construction", () => {
    const a = Atom.from(1);
    setAtomValidator(a, (v) => {
      return v > 0;
    });

    expect(a instanceof Atom<number>).toBeTruthy();
    expect(AtomStateProxy.setValidator).toHaveBeenCalledWith(a, expect.any(Function));
  });

  it("can define watchers which are called with the correct args on state change", () => {
    const a = Atom.from(1);
    const spy = jest.fn();
    addAtomWatcher(a, "test", spy);

    const swapRes = swap(a, (v) => v + 1);

    expect(swapRes.isOk()).toBeTruthy();
    expect(spy).toBeCalledWith(a["@@ref"], 1, 2);
  });
});