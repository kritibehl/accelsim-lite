function readX(o) {
  return o.x + 1;
}

const stableA = { x: 1, y: 2 };
const stableB = { x: 3, y: 4 };

for (let i = 0; i < 100000; i++) {
  readX(stableA);
  readX(stableB);
}

const changedShape = { y: 10, x: 20, z: 30 };

for (let i = 0; i < 1000; i++) {
  readX(changedShape);
}

console.log(readX(changedShape));
