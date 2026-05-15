# Hidden Class / Object Shape Observation Notes

## Scope

These notes compare AccelSim-Lite's toy object-shape runtime model to public V8 concepts.

This is not V8 implementation work, not JavaScript engine internals work, and not a claim of modifying V8.

## Public V8 concept

V8 documents Maps, also known as HiddenClasses, as runtime metadata that track the property layout of JavaScript objects. A map lists the properties associated with an object and describes where each property is located.

Reference:
- V8 docs: `https://v8.dev/docs/hidden-classes`

## Minimal JavaScript examples

### Same property order

```js
function makeA() {
  return { x: 1, y: 2 };
}

function makeB() {
  return { x: 3, y: 4 };
}

const a = makeA();
const b = makeB();

a.x;
b.x;
Expected conceptual behavior:

Objects are created with the same property order.
They are likely to share a compatible object shape / map conceptually.
Repeated access to x can remain shape-stable.
Different property order
function makeA() {
  return { x: 1, y: 2 };
}

function makeB() {
  return { y: 4, x: 3 };
}

const a = makeA();
const b = makeB();

a.x;
b.x;

Expected conceptual behavior:

Same property names can still produce different shape histories when insertion order differs.
A runtime property access site may see multiple shapes.
This maps to the monomorphic -> polymorphic transition modeled in AccelSim-Lite.
Shape growth
const obj = { x: 1, y: 2 };
obj.z = 3;
obj.x;

Expected conceptual behavior:

Adding a property changes the object's shape / map transition path.
A previously stable access site may observe a changed shape.
This maps to AccelSim-Lite's shape invalidation model.
AccelSim-Lite mapping
Observation	AccelSim artifact
Stable shape access	execution_engine_lab/object_shape_cache_sim.cpp
Multiple observed shapes	execution_engine_lab/inline_cache_transition_report.md
Shape invalidation	execution_engine_lab/property_access_profiles.json
Megamorphic-style fallback	execution_engine_lab/inline_cache_transition_report.md
Claims boundary

These notes only map public V8 concepts to AccelSim-Lite's educational runtime model.

They do not claim:

V8 implementation experience
V8 internals modification
production JavaScript engine work
production inline-cache development
