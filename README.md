write a readme for the quicksync library

# quicksync

`quicksync` is a library that provides a thread-safe and process-safe way to synchronize data between threads and processes inspired by Clojure Atoms. It is designed to be used in a multi-threaded or multi-process environment where data needs to be shared between threads or processes and to work with Node.js worker threads.

## Installation

```npm install quicksync```

## Usage

The `quicksync` public TS API exports the following:

- `Atom<T>` class
- `swap(atom: Atom<T>, fn: (state: T, ...args: any[]) => T, ...args: any[])`
- `deref(atom: Atom<T>)`
- `reset(atom: Atom<T>)`

An `Atom` is a thread-safe and process-safe container for a value. It can be used to synchronize data between threads and processes. It is similar to a Clojure Atom.

```ts
import { Atom } from 'quicksync';

const atom: Atom<number> = new Atom.from(0);
```

When an Atom is created, `quicksync` creates a reference to the atom in a shared memory location and maps it to an internally generated uuid. This uuid is used to identify the atom in the shared memory location. The uuid is also used to create a semaphore in a shared memory locatio. which synchronizes access to the atom.

Atoms do not allow direct updates to their state. Instead, we provide a `swap` method that takes an atom and a function that will be applied to the current state of the atom. The function will be called with the current state of the atom as its first argument and any additional arguments passed to `swap` as the remaining arguments. The function should return the new state of the atom.

```ts
const swapResult: true | false = swap(atom, (state, increment) => state + increment);

if (swapResult) {
    // swap was successful
} else {
    // swap was unsuccessful
}
```

Under the hood, this passes a reference to the atom stored in shared memory and a reference to the state update function to a NAPI process that is running in a separate thread. The NAPI process will then apply the state update function to the current state of the atom and save the new state of the atom to shared memory. The NAPI process will also acquire a lock on the atom's semaphore before applying the state update function and release the lock after the state update function has been applied, preventing dirty reads and writes from corrupting the atom's state is multiple threads or processes attempt to update the atom's state at the same time.

To reference the value of the atom, we provide a `deref` method that takes an atom and returns the current state of the atom.

```ts
const derefedValue: number = deref(atom);

// derefedValue === 1
```

Under the hood, this passes a reference to the atom stored in shared memory to a NAPI process that is running in a separate thread. The NAPI process will then return the current state of the atom and create a mutable copy of the state. It is important to note that mutating the deferenced copy of the atom state will not affected the atom's value in shared memory.

To reset the value of the atom, we provide a `reset` method that takes an atom.

```ts
reset(atom);

const derefedValue: number = deref(atom);

// derefedValue === 0
```

`reset` will set the atom's state back to the initial value the atom was created with.

## Limitations

`quicksync` currently supports JSON serializable values for the atom's state.

## License

`quicksync` is licensed under the MIT license. See the [LICENSE](LICENSE) file for more details.
