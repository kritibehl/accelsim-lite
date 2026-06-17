# Source Review Summary

## Changed RTL files

- `cad_flow/rtl/fifo.sv`
- `cad_flow/rtl/register_file.sv`

## Impacted checks

- fifo_push_pop
- fifo_full_empty
- fifo_ordering
- register_file_read_write
- register_file_address_bounds

## Reviewer focus

- FIFO pointer wraparound
- reset handling
- full/empty transition correctness
- register address bounds
- write/read ordering

## Risk level

`Medium`

## Recommendation

Run targeted RTL regression and review boundary behavior before merging.

## Claims boundary

This is an educational source-review artifact for CAD-flow automation practice.
