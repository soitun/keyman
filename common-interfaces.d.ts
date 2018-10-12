/**
 * A JavaScript string with the restriction that it must only
 * contain Unicode scalar values.
 * 
 * This means that any lone high surrogate must be paired with
 * a low surrogate, if it exists. Lone surrogate code units are
 * forbidden.
 * 
 * See also: https://developer.mozilla.org/en-US/docs/Web/API/USVString
 */
declare type USVString = string;

/**
 * TODO: ...
 */
declare interface Transform {
  insert: USVString;
  deleteLeft: number;
  deleteRight: number;
}

/**
 * TODO: ...
 */
declare interface Context {
  left: USVString;
  right?: USVString;
  startOfBuffer: boolean;
  endOfBuffer: boolean;
}