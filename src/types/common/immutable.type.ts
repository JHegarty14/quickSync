import { Primitive } from "./primitive.type";

export type Immutable<T> = T extends Primitive
  ? T
  : T extends Array<infer U>
  ? ReadonlyArray<U>
  : T extends Map<infer K, infer V>
  ? ReadonlyMap<K, V>
  : Readonly<T>;

export declare type DeepImmutable<T> = T &
  (T extends Primitive
    ? T
    : T extends Array<infer U>
    ? DeepImmutableArray<U>
    : T extends Map<infer K, infer V>
    ? DeepImmutableMap<K, V>
    : DeepImmutableObject<T>);

export interface DeepImmutableArray<T> extends ReadonlyArray<DeepImmutable<T>> {}

export interface DeepImmutableMap<K, V> extends ReadonlyMap<DeepImmutable<K>, DeepImmutable<V>> {}

export type DeepImmutableObject<T> = { readonly [K in keyof T]: DeepImmutable<T[K]> };
