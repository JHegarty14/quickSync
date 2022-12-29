import { Result, Err, Ok } from "./Result";

/**
 * @description Helper function to create an `Ok` variant of `Result`
 * @param value
 * @returns `value` wrapped in an `Ok` object
 */
export const ok = <T, E = never>(value: T): Ok<T, E> => new Ok(value);

/**
 * @description Helper function to create an `Err` variant of `Result`
 * @param err
 * @returns `err` wrapped in an `Err` object
 */
export const err = <T = never, E = unknown>(err: E): Err<T, E> => new Err(err);

/**
 * @description Wraps a function in a try/catch, and returns `Ok` if successful, `Err` if the function throws
 * @param fn Function to wrap with ok on success or err on failure
 * @param errorFn When an error is thrown, this will wrap the error result if provided
 * @returns Wrapped function result
 */
export const fromThrowable = <Fn extends (...args: readonly any[]) => any, E>(
  fn: Fn,
  errorFn?: (e: unknown) => E,
): ((...args: Parameters<Fn>) => Result<ReturnType<Fn>, E>) => {
  return (...args) => {
    try {
      const result = fn(...args);
      return ok(result);
    } catch (e) {
      return err(errorFn ? errorFn(e as E) : (e as E));
    }
  };
};
