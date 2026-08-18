# Ghosting test rigs

```
esphome run examples/test/ghost_test_4p2_v1_2.yaml
esphome run examples/test/ghost_test_5p79.yaml
```

Each cycles scene A → white → scene B → white every 12s. Look for residue on the
white frames, after 20-30 cycles, under bright light at a shallow angle.

Set `full_update_every: 9999` to characterise the partial waveform alone, then
`10` to check the periodic full refresh clears it.

The driver logs `Full`/`Partial refresh #N` and its duration. Similar durations
for both means the waveform swap isn't taking effect. On the 4.2" v1.2 it's
~1050 ms full vs ~450 ms partial.
