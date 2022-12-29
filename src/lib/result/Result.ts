/* eslint-disable @typescript-eslint/no-use-before-define */

/**
 * @class AbstractResult
 * @description Abstract class containing common functionality for Ok and Err
 */
abstract class AbstractResult<T, E> {
  /**
   * Used to check if a `Result` is an `OK`
   *
   * @returns `true` if the result is an `OK` variant of Result
   */
  isOk(): this is Ok<T, E> {
    return this instanceof Ok;
  }

  /**
   * @description Helper function to create an `Err` variant of `Result`
   * @param err
   * @returns `err` wrapped in an `Err` object
   */
  isErr(): this is Err<T, E> {
    return this instanceof Err;
  }

  abstract unwrap(): T | E;
}

/**
 * @class Ok
 * @description The `Ok` variant of `Result`
 * @param value The value to be wrapped in the `Ok` variant
 */
export class Ok<T, E> extends AbstractResult<T, E> {
  constructor(readonly value: T) {
    super();
  }

  /**
   * @description Returns the value wrapped in the `Ok` variant
   * @returns The raw value wrapped by the `Ok` variant
   */
  public unwrap(): T {
    return this.value;
  }
}

/**
 * @class Err
 * @description The `Err` variant of `Result`
 * @param err The error to be wrapped in the `Err` variant
 */
export class Err<T, E> extends AbstractResult<T, E> {
  constructor(readonly error: E) {
    super();
  }

  /**
   * @description Returns the error wrapped in the `Err` variant
   * @returns The raw error wrapped by the `Err` variant
   */
  public unwrap(): E {
    return this.error;
  }
}

export type Result<T, E> = Ok<T, never> | Err<never, E>;
