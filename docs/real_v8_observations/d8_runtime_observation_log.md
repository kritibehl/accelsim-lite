# d8 Runtime Observation Log

## Scope

This document records reproducible observation steps for connecting AccelSim-Lite runtime experiments to public V8 tooling concepts.

This is not a claim of V8 implementation or modification experience.

## Public V8 tooling concept

d8 is V8's developer shell for running JavaScript locally and debugging V8 behavior.

Reference:
- V8 d8 docs: https://v8.dev/docs/d8

## Local availability

Checked locally with:

d8 /tmp/shape_experiment.js

Observed result:

zsh: command not found: d8

So d8 was not available locally during this repo documentation pass.

## Example property-shape script

The intended JavaScript observation script is:

function sameOrderA() {
  return { x: 1, y: 2 };
}

function sameOrderB() {
  return { x: 3, y: 4 };
}

function differentOrder() {
  return { y: 5, x: 6 };
}

const a = sameOrderA();
const b = sameOrderB();
const c = differentOrder();

function readX(o) {
  return o.x;
}

for (let i = 0; i < 1000; i++) {
  readX(a);
  readX(b);
}

for (let i = 0; i < 1000; i++) {
  readX(c);
}

## Observation goal

| JavaScript behavior | Runtime concept | AccelSim toy model |
|---|---|---|
| same property order | stable shape / map | monomorphic cache |
| different property order | multiple shapes | polymorphic cache |
| many shape variants | generic fallback pressure | megamorphic fallback |
| added property | shape transition | shape invalidation |

## Claims boundary

This file documents public-tooling awareness and reproducible experiment ideas.

It does not claim:
- local V8 source build
- successful local d8 execution
- V8 patch contribution
- V8 internal debugging expertise
- production JavaScript engine development
