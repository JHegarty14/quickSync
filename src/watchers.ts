import { Atom } from "./atom";
import { AtomStateProxy } from "./atomStateProxy";
import { WatchFn } from "./types";

export function addAtomWatcher<T>(atom: Atom<T>, watcherKey: string, fn: WatchFn<T>): void {
    AtomStateProxy.addWatcher(atom, watcherKey, fn);
}

export function removeAtomWatcher<T>(atom: Atom<T>, watcherKey: string): void {
    AtomStateProxy.removeWatcher(atom, watcherKey);
}