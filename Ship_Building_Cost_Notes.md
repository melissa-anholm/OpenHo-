# Ship Building Cost Notes

> Provisional scratch specification. This document records the current placeholder ship-cost model while its internal consistency is being explored. It is not yet an implementation requirement or a final balance specification.

## 1. Scope

The first-pass cost model covers these ship types:

- Satellite
- Fighter
- Scout
- Colony ship
- Tanker
- Dreadnought

Biological ships are deferred until the game is otherwise mostly complete and therefore have no current cost parameters. Radical technology is also deferred. The ordinary technology streams in this model are:

- Range
- Speed
- Weapons
- Shields

Miniaturization is part of the cost pipeline, but its final formulas remain provisional. Every supported ship type permits every miniaturization level.

Money and metal are separate resources with separate formulas. A money formula must not be inferred by mechanically converting a metal formula, or vice versa.

## 2. Provisional Ship-Type Parameters

The base costs below apply at level 0 for all applicable technology streams. Technology multipliers are defaults shared across streams unless a stream-specific override is required by the ship's mechanics.

| Ship type | Base money | Base metal | Money tech multiplier | Metal tech multiplier |
| --- | ---: | ---: | ---: | ---: |
| Satellite | 1,000 | 100 | 0.4 | 0.4 |
| Fighter | 6,000 | 600 | 1.0 | 1.0 |
| Scout | 4,000 | 400 | 0.85 | 0.85 |
| Colony ship | 16,000 | 1,600 | 2.2 | 2.2 |
| Tanker | 12,000 | 1,200 | 1.8 | 1.8 |
| Dreadnought | 150,000 | 15,000 | 20.0 | 20.0 |

All values in this table are provisional. They should eventually be centralized as configuration parameters rather than embedded in formula logic.

## 3. Provisional Ordinary Technology Costs

For one level transition from `N` to `N + 1`, the first-pass placeholder base increments are:

| Technology stream | Money increment | Metal increment |
| --- | ---: | ---: |
| Range | 100 | 10 |
| Speed | 100 | 10 |
| Weapons | 100 | 10 |
| Shields | 100 | 10 |

These flat values are temporary. The eventual model may vary by stream, resource, and level transition. The implementation must therefore represent each transition as a function of `N`, rather than making the flat placeholder amount structurally permanent.

Each stream should have separately named money-cost and metal-cost formulas, even while the initial placeholder functions return the same value for every stream within a resource type. The level-transition argument must remain explicit, and formulas, parameters, multipliers, overrides, and locks should live in an adjustable formula/configuration layer.

Technology levels remain integer values in game state and at the public API boundary. Before any cost formula uses a level, the implementation must explicitly convert the relevant integer level or transition index to `double`.

For a resource `R`, ship type `S`, stream `T`, and final stream level `L`, the ordinary technology contribution is conceptually:

```text
ordinary technology cost(R, S, T, L)
    = sum for N = 0 through L - 1 of
      base increment(R, T, N)
      × multiplier(R, S, T)
```

`multiplier(R, S, T)` normally uses the ship type's resource-specific default from the table above, but may use a stream-specific override.

## 4. Ship-Type Exceptions

The multiplier model must support stream-specific overrides and technology locks:

- **Satellite speed:** Higher speed technology has no money or metal cost. The effective speed multiplier is zero.
- **Satellite range:** Satellite range is locked at level 0. Its multiplier is irrelevant for valid satellite designs because no higher range level can be selected; the lock should nevertheless be represented explicitly.

These are exceptions to the default shared-across-streams multiplier behavior.

## 5. Cost Calculation Pipeline

Miniaturization is applied after the base costs and all ordinary technology-stream costs have been combined. It is not another additive ordinary technology stream.

```text
pre-mini money cost = base money cost
                    + ordinary range money cost
                    + ordinary speed money cost
                    + ordinary weapons money cost
                    + ordinary shields money cost

pre-mini metal cost = base metal cost
                    + ordinary range metal cost
                    + ordinary speed metal cost
                    + ordinary weapons metal cost
                    + ordinary shields metal cost

final money cost = ceil(pre-mini money cost
                        × miniaturization money multiplier(M))

final metal cost = ceil(pre-mini metal cost
                        × miniaturization metal multiplier(M))
```

The miniaturization transformations must receive the complete pre-miniaturization total, not merely the base cost or only the ordinary technology increment. All intermediate calculations remain in `double`; the final public cost result is rounded upward to the next integer with `ceil`.

## 6. Provisional Miniaturization Rules

### Metal

Each miniaturization level multiplicatively scales the complete pre-miniaturization metal cost by `0.9`:

```text
miniaturization metal multiplier(M) = 0.9^M
final metal cost = pre-mini metal cost × 0.9^M
```

Examples:

```text
M = 0: 1.00 × pre-mini metal cost
M = 1: 0.90 × pre-mini metal cost
M = 2: 0.81 × pre-mini metal cost
```

The `0.9` factor is provisional and must remain adjustable.

### Money

For positive miniaturization levels, each level contributes a provisional additive multiplier of `3.0`:

```text
miniaturization money multiplier(0) = 1.0
miniaturization money multiplier(M) = 3.0 × M  (for M > 0)
final money cost = ceil(pre-mini money cost × miniaturization money multiplier(M))
```

Examples:

```text
M = 0: 1.00 × pre-mini money cost
M = 1: 3.00 × pre-mini money cost
M = 2: 6.00 × pre-mini money cost
```

The `3.0` contribution is provisional and will probably be replaced by a formula that depends explicitly on miniaturization level. The `M = 0` identity multiplier of `1.0` is confirmed. The displayed multiplications are conceptual; the calculation remains in `double` until the final upward rounding step.

## 7. Prototype Cost

The first ship built from a design incurs a provisional money-only prototype charge:

```text
prototype cost = ceil(10 × unrounded resulting money build-cost expression)
```

This 10× multiplier is placeholder behavior and is expected to be replaced by the final rule. The prototype charge does not create a separate metal charge. Prototype calculation must likewise retain `double` precision until its own final upward rounding step; it should not truncate or round the underlying money calculation first.

## 8. Implementation Requirements and Remaining Questions

The implementation should:

- Keep money and metal formulas separate.
- Keep each ordinary technology stream's level-transition function separately replaceable.
- Keep the level `N` argument explicit in ordinary technology formulas.
- Support ship-type defaults, stream-specific multiplier overrides, and technology locks.
- Apply miniaturization after all base and ordinary technology costs.
- Keep the `0.9` metal factor and `3.0` positive-level money contribution adjustable.
- Keep all intermediate ship-cost calculations in `double` and round each public final cost upward with `ceil`.
- Centralize all placeholder values in configuration or formula parameters.

Still unresolved for the final model:

- The final level-dependent formula for each ordinary technology stream and resource.
- Final ship-type multipliers and any additional stream-specific overrides.
- The final metal miniaturization factor or function.
- The final money miniaturization function.
- The final prototype-cost rule.
- Any additional technology locks or ship-specific exceptions.
