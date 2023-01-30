export type WatchFn<T> = (ref: string, oldState: T, newState: T) => void;