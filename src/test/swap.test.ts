import { Atom, deref, swap } from "../../src";

describe("swap function", () => {
  it("is a function", () => {
    expect(swap).toBeInstanceOf(Function);
  });

  it("applies the passed-in fn to the Atom's value and sets the Atom's value to the return value", () => {
    const initialState = { count: 0 };
    const nextState = { count: 1 };
    const testAtom = Atom.from(initialState);
    expect(deref(testAtom)).toEqual(initialState);

    const swapResult = swap(testAtom, s => nextState);

    expect(swapResult.isOk()).toBe(true);
    expect(swapResult.unwrap()).toBeNull();

    expect(deref(testAtom)).toEqual(nextState);
  });

  it("applies the passed function and optional args to the Atom's value and returns the correct updated value", () => {
    const initialState = { count: 1 };
    const testAtom = Atom.from(initialState);

    const swapResult = swap(testAtom, (s, n) => ({ count: s.count + n }), 2);

    expect(swapResult.isOk()).toBe(true);
    expect(swapResult.unwrap()).toBeNull();

    expect(deref(testAtom)).toEqual({ count: 3 });
  });

  it("works with multiple atoms; not mixing up their states", () => {
    const a = { nums: [1, 2, 3, 4, 5] };
    const b = 9;
    const c = { hi: "hello" };
    const testAtomA = Atom.from(a);
    const testAtomB = Atom.from(b);
    const testAtomC = Atom.from(c);
    expect(deref(testAtomA)).toEqual(a);
    expect(deref(testAtomB)).toEqual(b);
    expect(deref(testAtomC)).toEqual(c);

    swap(testAtomA, s => ({ nums: s.nums.map(n => n + 1) }));
    const a1 = { nums: [2, 3, 4, 5, 6] };
    expect(deref(testAtomA)).toEqual(a1);
    expect(deref(testAtomB)).toEqual(b);
    expect(deref(testAtomC)).toEqual(c);

    swap(testAtomB, x => x + 1);
    const b1 = 10;
    expect(deref(testAtomA)).toEqual(a1);
    expect(deref(testAtomB)).toEqual(b1);
    expect(deref(testAtomC)).toEqual(c);

    swap(testAtomC, s => ({ hi: s.hi.toUpperCase() }));
    const c1 = { hi: "HELLO" };
    expect(deref(testAtomA)).toEqual(a1);
    expect(deref(testAtomB)).toEqual(b1);
    expect(deref(testAtomC)).toEqual(c1);
  });
});