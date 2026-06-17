# Failure Injection Report

## Scope

This report documents a deliberately broken RTL example used for CAD-flow failure-mode discussion.

## Artifact

- `cad_flow/examples/broken_fifo.sv`

## Injected failure

The example has intentionally incomplete FIFO behavior:

- no storage array
- no read/write pointer management
- no count tracking
- unrealistic full/empty behavior

## Expected review outcome

A real RTL review should flag this as unsafe for functional validation.

## Claims boundary

This is a failure-injection documentation artifact for educational CAD-flow automation. It is not a production RTL verification result.
