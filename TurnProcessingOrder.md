# Turn Processing

**Status:** Approved design direction. The economic path, readiness/submission gate, and queued abandonment transition are implemented; battle resolution, automatic fleet-triggered colonization, and general ordered player-action execution remain future work.

This document defines the intended order of economic processing, queued fleet actions, player actions, and state persistence. It distinguishes authoritative economic results from later projections.

## 1. Lifecycle and snapshot conventions

The canonical snapshot labeled **Turn N** is the state at the beginning of Turn N, before that turn's economic processing. The first turn is **Turn 0**, beginning in year **2000**. `current_turn` identifies the next economic turn awaiting processing; therefore the current accepted economic submission targets `current_turn`. `current_year` is separate state and currently advances by 10 years per processed turn.

The approved eventual turn proceeds in this order:

1. **Beginning-of-turn actions** — apply actions scheduled for the start of the new turn, including colony abandonment.
2. **Economic processing** — calculate and apply the current turn's population, planetary income, interest, allocations, research, mining, and terraforming.
3. **Queued fleet actions** — resolve travel, battles, and automatic colonization.
4. **Player actions** — execute immediate actions and queue actions for the following turn.
5. **End-turn processing** — persist state, write history, and prepare the next turn.

The canonical Turn N snapshot is the input to that turn's economic phase. The economic after-state is the committed result of that phase. The post-processing state is the live state after queued fleet actions and player actions, from which the next turn proceeds after its beginning-of-turn actions are applied.

### Implemented processing path

The current core implementation follows this narrower path:

1. **Readiness gate** — require every non-eliminated player to have a valid economic submission targeting `current_turn` and to have explicitly marked themselves done.
2. **Economic processing** — lock pending Dip Into Savings, capture opening savings, capture public information, grow population, refresh planetary income, calculate income and interest, apply Dip Into Savings, allocate money, research, and planet development.
3. **Provisional post-economic processors** — process fleet arrivals through `process_ships()` and run the no-op nova placeholder. Battles and automatic fleet-triggered colonization are not yet implemented. Beginning-of-turn abandonment is implemented through `process_queued_abandonments()`.
4. **View refresh** — refresh `PlanetView` snapshots after the current processors settle.
5. **Turn boundary** — check the provisional bankruptcy boundary, capture `EconomicAfterState` as Turn N, advance `current_turn` and `current_year`, renew persistent dips, capture submissions for the new current turn, and clear readiness.

The current implementation does not yet execute a separate general player-action phase inside `process_turn()`. Player-facing economic choices and abandonment requests are submitted and accepted before readiness; changing one clears readiness and requires the player to mark themselves done again. Gameplay colonization is available as a GameState transition for future fleet/battle code, but is not a direct player action.

### State boundaries

**Beginning-of-turn snapshot:** current turn/year; planet ownership, population, temperature, actual gravity, metal, and income inputs; player savings, metal reserves, technologies, research, and budget allocations from the previous turn.

**Economic after-state:** population after growth; planetary income and interest used for the turn; net/effective income; savings; metal reserve; and per-colony population/income records. The broader runtime checkpoint also stores planet metal/temperature and technology/research state.

**Post-processing state:** changes from combat, colonization, abandonment, ship/fleet actions, and immediate balance changes, plus recalculated projected interest and planetary income.

The economic after-state is retained for explaining and debugging the completed turn. Public and private history views use the same snapshot boundary but apply different knowledge filters. Later phases may update persistent balances and view-only projections, but cannot retroactively alter the committed economic after-state.

The completed-turn history record retains the beginning-of-turn Turn N/year label. The next canonical snapshot is Turn N+1 with the newly advanced year.

## 2. Economic processing

Economic processing reads the beginning-of-turn snapshot. For the first turn, explicit starting values must be established before this phase runs.

### 2.1 Population and planetary income

For each owned planet, calculate population growth using beginning-of-turn population, beginning-of-turn temperature, actual gravity, and the owning player's relevant conditions or ideals. Apply growth before calculating current-turn planetary income.

Calculate each planet's income once for current-turn economics using:

- Population after growth.
- The planet temperature present when income is refreshed during economic processing.
- Actual gravity.
- Current ownership.
- The owning player's relevant conditions or ideals.

Store that value as the authoritative income for the turn. Do not recalculate authoritative current-turn income after mining, terraforming, combat, colonization, or player actions. Thus, new population contributes to the current turn. The implementation currently performs planet development after income refresh, so development changes do not alter the already-calculated current-turn income.

This document uses actual gravity consistently; distinctions among actual, effective, apparent, and perceived gravity may be defined elsewhere.

### 2.2 Interest and net income

Calculate current-turn interest from beginning-of-turn money savings:

```text
turn_interest = interest(beginning_of_turn_savings)
```

Add this value to the current income pool. It is not directly added to savings, and it is not applied a second time during the same economic phase.

First calculate net income:

```text
net_income = sum(current_turn_planetary_income) + turn_interest
```

Dip Into Savings is selected during the player-action phase for a future
economic phase. It has an immediate savings effect and a queued economic
effect. The current-turn selection is provisional until the player-action
phase ends.

The action maintains separate pending and locked values:

```text
pending_dip_amount       = current reversible selection
locked_dip_amount        = amount committed for the next economic phase
pending_dip_persistent   = whether the selection renews automatically
locked_dip_persistent    = persistence mode of the committed action
```

When the player changes the pending selection, savings are adjusted only by
the difference between the old and new selections:

```text
savings += old_pending_dip_amount - new_pending_dip_amount
```

Thus, selecting, changing, or cancelling a dip repeatedly during one player
action phase cannot create or destroy money. The pending selection does not
affect the current turn's net or effective income. When the player-action
phase ends, the pending selection becomes the locked action for the next
economic phase; savings are not adjusted a second time at lock time.

At the next economic phase, the locked dip is added to net income:

```text
effective_income = net_income + locked_dip_amount
```

The locked dip has already been removed from savings during the preceding
player-action phase, so economic processing does not subtract it again. A
requested dip is capped immediately at the current permitted intentional-debt
amount. By default, selecting a positive dip creates a persistent **pension
plan** that renews automatically for later turns. Each renewal is re-evaluated;
if less borrowing capacity is available, the stored recurring request is
reduced to the newly permitted amount and that smaller amount is used for later
renewals. The player may instead select a one-time dip, which is cleared after
its locked amount is applied. Setting the pending dip to zero explicitly
disables a pension plan.

### 2.3 Effective income and allocations

When effective income is positive, apply the accepted current-turn submission's fractional allocations:

```text
allocated_money = effective_income × allocation_fraction
```

The allocation categories are savings, planetary development, and technology/research. Planetary development is divided among owned planets, then between terraforming and mining. Technology/research is divided among research streams. Every category uses the same effective-income value.

When effective income is negative, there is no money to distribute:

- No savings, planetary-development, or research allocation is funded.
- The negative amount is charged directly against money savings:

```text
savings_after_economic_processing = savings_at_economic_start + effective_income
```

When effective income is positive, the savings allocation is included in the
same formula:

```text
savings_after_economic_processing = savings_at_economic_start
                                    + effective_income × savings_allocation_fraction
```

Here `savings_at_economic_start` already includes the immediate deduction
made when the locked dip was selected. Across both phases, the positive-income
case is therefore equivalent to:

```text
opening_savings - dip_amount
    + effective_income × savings_allocation_fraction
```

Savings may therefore become debt. A single negative-income turn does not automatically cause bankruptcy; recovery remains possible. Completed-turn processing performs one check against the provisional bankruptcy limit. The check currently records detection only; bankruptcy consequences and loss of the game remain a later implementation task.

### 2.4 Apply economic changes

Apply the allocations and persist the resulting changes to savings, accumulated metal reserve, research and technology, planet temperature, and planet metal.

#### Mining

Mining requires positive effective income allocated to planetary development. The transfer is lossless:

```text
planet_metal -= metal_extracted
player_metal_reserve += metal_extracted
```

Planetary metal may reach zero but may not become negative. Once depleted, a planet cannot yield more metal. The player's metal reserve is shared across owned planets and shipyards. Mining expenditure and metal transfer occur within the same economic operation and neither depends on the other's result.

#### Terraforming

Terraforming consumes money but not metal. Its nonlinear temperature change takes effect during economic processing. If the allocation would pass the owner's target temperature:

- Stop at the target.
- Charge only the amount needed to reach it.
- Refund the unused allocation immediately to money savings.

The current implementation uses this refund destination provisionally; the final destination remains a future design decision. Current-turn planetary income remains fixed after section 2.1.

### 2.5 Economic after-state

At the current implementation's turn boundary, the captured economic after-state contains:

- Population after growth.
- Planetary income, interest, and net/effective income used for the turn.
- Savings and accumulated metal reserve after economic processing.
- Per-colony population and income.

The broader runtime checkpoint separately stores planet metal and temperature, technology and research progress, and the final allocations carried forward to the next turn.

## 3. Post-economic projections

After economic processing, distinguish authoritative current-turn values from display-only recalculations.

| Calculation | Input | Changes completed current-turn economics? |
|---|---|---:|
| Turn-processing interest | Beginning-of-turn savings | Yes |
| Displayed/projected interest | Current savings after later actions | No |
| Turn-processing planetary income | Population after growth, beginning temperature, actual gravity, ownership, player conditions | Yes |
| Displayed/projected planetary income | Current post-processing population, ownership, temperature, and other inputs | No |

Recalculate and display projected interest whenever later actions change savings, including construction and transfers. Recalculate projected planetary income when later actions change ownership, population, temperature, or another income input. Do not cache either projection as an authoritative value between turns; the next economic phase recalculates from current state.

Post-economic combat, colonization, abandonment, and player actions may change persistent balances and projections, but cannot change the already-committed economic income or allocations for the current turn.

## 4. Queued fleet actions

Queued fleet actions resolve after economic processing. The implemented portion applies queued travel, deferred older-fleet scrapping, colony-ship replenishment immediately after movement, fleet-arrival processing, and a basic automatic-colonization pass for currently stationed fleets containing a loaded colony ship; battle resolution and battle-aware colonization remain future phases.

**Implementation status:** Queued movement submission, readiness validation, travel initiation, deferred older-fleet scrapping, arrival processing, per-ship colony capacity, replenishment, and loaded-colony-ship qualification are implemented. The current `process_ships()` path does not resolve battles; automatic colonization therefore runs after arrival processing but before future battle integration.

An empty colony ship replenishes only after the current turn's queued fleet-movement phase has completed. It therefore cannot replenish and then depart during the same movement phase; it must remain stationed through that phase and can move no earlier than the following turn.

### 4.1 Fleet travel

Each fleet performs its queued travel action. After travel, it is either at a specific planet or still in transit. Fleets still in transit have no further queued-fleet action that turn.

### 4.2 Battle resolution

Whenever forces belonging to two or more non-allied players occupy the same place, a battle occurs. Fleets are forces, and an owned planet is also a force. Thus, a fleet and a defending planet are not separate categories of battle; they are forces participating in the same battle.

The planet may not be targeted by invading weapons until all defending ships have been destroyed. It remains an active force throughout the battle and may fire at attackers when its turn comes around. Once the planet itself is targeted, its population is destroyed as determined by the battle. When all population has been destroyed, the colony is destroyed and the planet becomes unowned.

Exact battle mechanics remain open, including damage, casualties, technology effects, force-turn order, three-or-more-player battles, victory/defeat, and retreat. The intended outcomes are:

- Battle continues until at least one side loses all relevant forces.
- Surviving damaged ships are restored to full strength when the battle concludes.
- If opposing sides completely destroy one another, the result is a draw.

There is no separate planet-capture operation. Combat population loss immediately reduces displayed/projected income, but does not alter income already processed for the current turn. Any unextracted metal remains on the planet after colony destruction for whoever colonizes it later.

### 4.3 Automatic colonization

**Implementation status:** `GameState::colonize_planet()` provides the unified gameplay transition, and the current turn path automatically invokes it for a currently stationed fleet containing a loaded `SHIP_COLONY` design at an unowned planet. Exactly one ship’s 10 colonists are consumed; other colony ships retain their own states. Ordinary fleets cannot trigger this path. Battle resolution and battle-aware ordering remain future work.

After battles, a ship of the colony-ship design arriving at an uncolonized planet automatically establishes a colony. A colony ship is an ordinary ship of a particular design selected during construction, not a separate universal object.

- Colonization is free; no ship or resource is consumed.
- The projected income immediately shows the colony's current support cost, while economic effects begin with a later economic phase.
- Multiple colony ships from the same player retain independent 10/0 colonist states; only one loaded ship is consumed when a planet is colonized.
- An empty colony ship replenishes to 10 after spending a turn stationed at a colonized planet.
- Non-allied colony ships must fight before colonization.
- If eligible colony ships from multiple allied players are present, the globally fastest individual colony ship proceeds; if several ships tie, the deterministic game RNG selects one. If only one player has eligible colony ships, that player's slowest individual colony ship proceeds; equal-speed ties are randomized.
- A colony ship that survives a battle may colonize during that same turn.
- Planet desirability is calculated for immediate UI use when a player explores a planet; that initial rating need not be stored. If colonized, recalculate and store the rating with `ColonizedPlanet`, then recalculate it during later turn processing.

## 5. Player actions

Player actions occur after queued fleet actions. Immediate actions execute in the order specified by the player. Persistent balance changes take effect immediately, affect later same-phase actions, and do not alter the completed economic after-state.

**Implementation status:** The current core has no separate ordered player-action executor. Economic choices are submitted as one final non-ordered `EconomicActionSubmission` before readiness, and the server processes them only after the readiness gate opens. Resource transfers and other ordered action lists remain deferred.

### 5.1 Ship construction and fleet assembly

Building a ship costs money and metal, charges both immediately, and creates the ship immediately. The first ship of a given type also incurs a money-only prototyping cost, expected to be approximately ten times the individual ship's money construction cost until final formulas and data are established. Multiple ships may be built if each action is affordable when attempted. An unaffordable action fails atomically.

A fleet is assembled from ships that have already been built; it is not a separately built resource-consuming object. A complete newly built stationed fleet may be scrapped immediately during the current player-action phase, with construction money and metal refunded and a first-use prototype marker rolled back when no ships of that design remain. Older stationed fleets may be marked for scrapping during player actions; the request is processed during the following turn's queued-fleet phase and refunds only `floor(0.15 × metal_build_cost_of_fleet)` metal. Fleets in transit cannot be scrapped. The 15% factor remains provisional and requires data-driven calibration.

### 5.2 Resource transfers

A player may immediately send money or metal to another player. Transfers are lossless and update both participants' persistent balances, so later same-phase actions see the changes. A player may not send more metal than the current reserve or enough money to go below the permitted savings minimum/exceed permitted debt. Projected interest and other derived displays may update immediately. Resource transfers are intentionally low priority for implementation.

### 5.3 Actions queued for the next turn

In the approved future model, examples include Dip Into Savings, fleet movements, colony abandonment, and diplomatic actions whose rules eventually resolve later. In the current implementation, Dip Into Savings is the principal queued action, while the remaining economic choices are submitted as one final non-ordered snapshot. Dip selection immediately updates the persistent savings balance but queues the income effect; its pending amount and persistence mode remain mutable until readiness freezes the submission.

### 5.4 Colony abandonment

Abandonment is queued during player actions and applied at the beginning of the next turn's economic processing, before that turn's income is calculated. It is free:

- The colony and its colonists are removed from the player's owned-colony state.
- Any unmined metal remains on the planet for a future colonizer.
- Fleets stationed at the planet remain there.
- Projected income updates immediately when applicable.
- The abandoned colony contributes neither income nor support costs to the next economic phase.

### 5.5 Budget changes

Budget allocations carry forward by default. Changes made during player actions apply to the next economic phase, not the phase already completed.

## 6. Readiness and persistence

The core turn-processing entry point enforces readiness; callers cannot bypass this check.

- All non-eliminated players begin unready.
- A player may ready, unready, change actions, and ready again before processing begins.
- Any action submitted after readying makes the player unready. Immediate actions remain possible after readying; whether the player is automatically re-readied afterward or must explicitly ready again remains open.
- Queued actions are mutable until processing begins and are frozen for the processing pass once the turn advances.
- Eliminated players are excluded from readiness and have no actions.
- If a player disconnects, the host must explicitly decide whether to proceed. If the host disconnects, the new host inherits that responsibility.
- An AI becomes ready only after completing its planned actions. A configurable turn-length limit must prevent an AI loop from blocking the game indefinitely.
- When all required players are ready and any disconnect decision is resolved, the engine processes the turn atomically and resets readiness for the next turn.

The current runtime checkpoint persists the implemented economic/planet state, PlanetView snapshots, RNG, players, technologies, balances, and turn/year values. Fleet state, readiness state, pending ordered actions, and complete public-information history remain outside the currently supported resumable checkpoint. The intended future policy is to persist the complete resumable state, synchronize critical state for host migration at least once per turn, and write history using the beginning-of-turn Turn N/year label before advancing to Turn N+1.

## 7. Open decisions

1. Data-dependent lower limit for negative savings and the exact bankruptcy rule.
2. Immediate versus queued diplomatic actions; diplomacy is intentionally deferred.
3. Final ship-design, construction, and scrapping cost formulas, including the typical prototyping-cost ratio.
4. Remaining money and metal flows for combat.
5. Final serialization coverage and compatibility/versioning policy for complete mid-game resumes.
