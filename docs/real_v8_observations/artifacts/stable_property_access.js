function readX(o) {
  return o.x;
}

const a = { x: 1, y: 2 };
const b = { x: 3, y: 4 };

for (let i = 0; i < 100000; i++) {
  readX(a);
  readX(b);
}

console.log(readX(a));
