# Turn Processing

**Status:** Design draft; not yet the final source of truth.

This document defines the intended order of economic processing, queued fleet actions, and player actions. It distinguishes values that affect the current turn's economics from values recalculated for display or for the next turn.

The canonical snapshot labeled **Turn N** is the state at the beginning of Turn N, before that turn's economic processing. The first turn is **Turn 0**, and the starting year is **2000**. `current_turn` and `current_year` are separate state variables; the mapping for later years remains a separate game-rule value.

## 1. Turn phases

Each turn proceeds through these phases, in order:

1. **Economic processing** — apply the current turn's population, income, interest, allocation, research, mining, and terraforming effects.
2. **Queued fleet actions** — resolve actions queued by players during the previous turn, including movement, battles, colonization, population loss, and colony loss or ownership changes.
3. **Player actions** — process immediate actions and queue actions for the following turn.
4. **End-turn processing** — store and log the state needed for history, display, and the next turn.

The economic phase is the only phase that applies the complete current-turn economic model. Later phases may update persistent balances immediately and may recalculate derived economic values for display, but those derived values do not retroactively alter the completed economic phase.

## 2. State terminology

The design uses three distinct state boundaries.

### Beginning-of-turn state

This is the canonical `Turn N` snapshot and the input to the current economic phase. It includes:

- Current year and turn.
- Planet ownership.
- Planet population.
- Planet temperature.
- Planet actual gravity.
- Planet metal.
- Planetary income inputs or cached displayed income.
- Player money savings.
- Player accumulated metal reserve.
- Player technology levels and research progress.
- Player fractional budgetary allocations, as set by the player during the previous turn.

### Economic after-state

This is the committed result of the current economic phase. It includes:

- Population after current-turn growth.
- Planetary income used for the current turn.
- Interest used for the current turn.
- Net/effective income used for the current turn.
- Savings after the current-turn savings allocation.
- Metal after mining and other economic effects.
- Technology and research progress after research processing.
- Temperature after terraforming.

### Post-processing interactive state

This is the state after queued fleet actions and player actions. It may include:

- Changed ownership.
- Newly colonized or abandoned planets.
- Population loss from combat or other actions.
- Newly built or destroyed ships and fleets.
- Immediate changes to money savings.
- Immediate changes to accumulated metal reserve.
- Recalculated projected interest and planetary income for display.

Money savings and accumulated metal reserve are persistent action-state balances. Other economic values recalculated after economic processing are normally view-only projections until the next economic phase.

The economic after-state is retained for explaining and debugging the current turn. The post-processing interactive state is the live state from which the next turn's beginning-of-turn processing proceeds, after any actions explicitly scheduled for the beginning of that next turn have been applied. Public and private history views should be derived from the same snapshot boundary and filtered according to each player's knowledge.

## 3. Economic processing

### Information available before processing begins

For the first turn, explicit starting values must be established before this phase is run. The exact starting values remain an open design question.

#### Global values

- Current year and turn.
- The current year remains constant while the economic phase runs. Turn/year advancement occurs during end-turn processing, subject to the final snapshot rule described below.

#### Planet values

For each planet:

- Ownership.
- Population.
- Temperature.
- Actual gravity.
- Metal.
- Any cached or displayed planetary-income value.

#### Player values

For each player:

- Total money savings.
- Total accumulated metal reserve.
- Technology levels and research progress.
- Fractional budgetary allocations selected by the player during the previous turn.

### 3.1 Calculate population changes

For each owned planet, calculate population growth using the beginning-of-turn values unless a later rule explicitly says otherwise:

- Beginning-of-turn population.
- Beginning-of-turn temperature.
- Planet actual gravity.
- The owning player's relevant conditions or ideals.

Apply the resulting population changes before calculating the planetary income used by the current turn.

For this document, use actual gravity consistently. The distinction between actual, effective, apparent, and perceived gravity may be defined elsewhere.

### 3.2 Calculate current-turn planetary income

For each owned planet, calculate planetary income once for use by the current economic phase. The calculation uses:

- Newly calculated population.
- Beginning-of-turn temperature.
- Planet actual gravity.
- Current ownership.
- The owning player's relevant conditions or ideals.

The resulting planetary income is stored as the value used for this turn's income calculation. Do not recalculate authoritative current-turn planetary income again after mining, terraforming, combat, colonization, or player actions.

This means newly grown population contributes to the current turn's income, while temperature changes caused by current-turn terraforming affect economic income beginning with a later turn.

### 3.3 Calculate turn-processing interest

Calculate interest for current-turn economics from the money-savings balance at the beginning of the turn:

```text
turn_interest = interest(beginning_of_turn_savings)
```

This value is added to the current turn's income pool. It is not directly added to the savings account, and interest must not be applied a second time to the savings balance during the same economic phase.

### 3.4 Calculate net and effective income

For each player:

```text
net_income = sum(current_turn_planetary_income) + turn_interest
```

Until the **Dip Into Savings** feature is implemented:

```text
effective_income = net_income
```

When effective income is positive, it is distributed according to the player's budget allocations. When effective income is negative, there is no money to distribute:

- No money is allocated to savings.
- Planetary development is not funded.
- Technology research is not funded.
- The negative effective income is charged directly against the player's money savings account.
- Savings may therefore become negative and represent debt.

The player does not automatically go bankrupt merely because one turn has negative effective income. The player may recover later if planetary income becomes positive or other rules improve the net-income situation. There is a data-driven lower limit for negative savings; reaching that limit causes bankruptcy and loss of the game. The limit remains to be determined.

### 3.5 Apply budgetary allocations

Use the player's fractional budgetary allocations selected during the previous turn to apply this turn's effective income. The allocations are divided into:

- Savings.
- Planetary development.
- Technology/research.

The planetary-development allocation is split among owned planets. Each planet's allocation is split between terraforming and metal extraction. The technology allocation is split among individual research streams.

When effective income is positive, for every allocation category:

```text
allocated_money = effective_income × allocation_fraction
```

All allocation categories for a given turn must use the same effective-income value.

When effective income is negative, the allocation base for all distributable categories is zero. The negative amount is handled as a direct reduction of money savings under section 3.6.

### 3.6 Apply economic changes

Apply the allocated amounts and calculate the resulting changes:

- Change in money savings.
- Change in accumulated metal reserve.
- Change in technology levels and research progress.
- Change in planet temperature.
- Change in planet metal.

For negative effective income, do not apply a negative savings allocation. Instead:

```text
savings_after = opening_savings + effective_income
```

Because `effective_income` is negative in this case, this directly increases the player's debt or reduces positive savings. If savings reaches the data-driven bankruptcy limit, the player loses the game.

#### Mining

Mining is funded only from positive current-turn effective income allocated to planetary development. A player with negative effective income cannot mine before **Dip Into Savings** exists, because there is no positive money available to allocate to mining.

For each mining operation during economic processing:

```text
planet_metal -= metal_extracted
player_metal_reserve += metal_extracted
```

The transfer is lossless. Planetary metal may reach zero but may not become negative; once a planet is depleted, no additional metal can be extracted from it. The player's metal reserve is shared across all owned planets and shipyards. The mining expenditure and metal transfer occur in the same economic operation; neither depends on the result of the other.

#### Terraforming

Terraforming consumes money from the positive current-turn planetary-development allocation. It does not consume metal. The temperature change is a nonlinear function of money spent and takes effect immediately during economic processing.

If the allocated budget would move the planet beyond the owner's target temperature:

- Stop at the target temperature.
- Charge only the amount needed to reach the target.
- Refund the unused allocation immediately to money savings.

The current implementation already performs overshoot prevention and refunds the excess to `money_savings`. The destination of this refund is provisional and requires a future design decision.

Mining and terraforming must avoid overshoot and refund any excess allocation according to their finalized rules.

Do not recalculate current-turn planetary income after these changes. The economic income used for this turn is already fixed by section 3.2.

### 3.7 Economic after-state

At the end of economic processing, persist:

#### Player values

- Money savings after the current-turn savings allocation.
- Accumulated metal reserve after current-turn economic processing.
- Metal change for the turn.
- Technology levels and research progress.
- Fractional budgetary allocations, unchanged unless a rule explicitly updates them.
- Net/effective income used for this turn.

#### Planet values

- Ownership, unchanged by ordinary economic processing.
- Population after growth.
- Temperature after terraforming.
- Metal after mining.
- Planetary income used for this turn, calculated from new population and old temperature.

## 4. Interest and derived-value display after economic processing

Interest is calculated for two different purposes.

### Current-turn economic interest

This is the value calculated from beginning-of-turn savings and included in current-turn net income. It is part of the committed economic after-state.

### Displayed/projected interest

After economic processing, calculate interest from the current savings balance and show the result to the player:

```text
displayed_interest = interest(current_savings)
```

Recalculate and display this value whenever player actions change savings, including construction, transfers, or other applicable actions. This displayed value does not alter the already-completed current-turn economics.

There is no need to cache the value between turns. At the beginning of the next economic phase, calculate turn-processing interest again from the then-current savings balance.

### Displayed/projected planetary income

If fleet actions or player actions change ownership, population, temperature, or another income input, recalculate planetary income for display as needed. These post-processing calculations are view-only projections until the next economic phase.

There is no need to carry a planetary-income cache as an authoritative value between turns. Population and other inputs will be processed again before income is used economically in the next turn.

## 5. Queued fleet actions

Resolve actions queued during the previous player-action phase after economic processing has completed. These may include:

- Fleet movement.

### 5.1 Fleet travel

First, every fleet performs its queued travel action, if any.

After the travel phase, each fleet is either:

- Located at a specific planet; or
- Still in transit through space.

Fleets that remain in transit have no further actions during this turn's queued-fleet-actions phase.

### 5.2 Battle resolution

Fleets located at a planet after travel must resolve all pending battles at that planet:

- Fleets from two or more non-allied players at the same planet must fight one another.
- A fleet stationed at a planet owned by a non-ally must fight the planet and its defending colony.

The exact battle rules remain to be defined. Eventually, battles may destroy some or all ships and, when a planet is involved, may kill some or all of the planet's population. Battle resolution continues until at least one side loses all of its forces, meaning its fleets and, where applicable, its planet/colony forces.

The current intended outcomes include:

- If all population on a planet is killed, the colony is destroyed and the planet becomes unowned until colonized again.
- Ships damaged but not destroyed during a battle are immediately restored to full strength when that battle concludes.
- If both sides completely destroy one another's forces, the battle ends in a draw rather than producing a specific winner.
- Battles involving three or more players require a future multi-party battle-resolution design.

### 5.3 Automatic colonization

After battles have been resolved, a ship of the colony-ship design that arrives at an uncolonized planet automatically establishes a colony there. A colony ship is not a separate universal object: it is a particular ship type selected by the player during ship construction.

- Establishing the colony at this point is free.
- The colony ship and other ships are not consumed or lost.
- No resources are consumed by the act of colonization.
- The player's projected income is updated immediately to show the colony's current support cost, even though the colony's economic effect is not applied until the following economic phase.
- The planet's desirability is calculated for the newly explored planet for immediate UI use; once colonized, the rating is recalculated and stored with the `ColonizedPlanet` state for subsequent turn processing.
- Multiple colony ships arriving together produce no special result when they belong to the same player.
- Colony ships belonging to different non-allied players must fight before colonization can occur.
- If allied colony ships arrive together, their speed technology rating determines which ship proceeds to colonization.
- A colony ship that survives a battle may still colonize during that same turn.

At present, no other queued fleet actions have been identified.

These travel and battle actions may immediately change the world state and may immediately recalculate derived economic values for display. Their economic effects generally become inputs to the following turn's economic processing.

There is no separate planet-capture operation. If combat destroys a colony, the planet becomes unowned, its remaining metal stays available on the planet, and a later arriving colony ship may establish a new colony there.

Combat population loss immediately reduces the displayed/projected income of the affected colony, but the reduced population does not alter the economic income already processed for the current turn. Its effect is normally felt during the following economic phase.

## 6. Player actions

Player actions occur after queued fleet actions. They may be immediate or queued for the next turn.

### Immediate actions

Examples include:

- Building ships.
- Adjusting fleets.
- Sending resources.
- Other actions that directly change persistent balances.

When multiple immediate-effect actions are issued during the player-action phase, they are applied in the order specified by the player. There is no separate fixed engine ordering for those actions.

Ship-building and resource-transfer costs are applied immediately. Money savings and accumulated metal reserve therefore update immediately and affect which further actions are available during the current turn.

### Ship construction and fleet assembly

Ship construction is an immediate player action:

- Building a ship costs both money and metal.
- The money and metal charges are applied immediately.
- The ship is created immediately.
- The first ship of a given type also incurs a money-only prototyping cost.
- The prototyping cost is expected to be approximately ten times the money cost of building one individual ship of that type, subject to the eventual cost formulas and data.
- Multiple ships may be built during one player-action phase, provided each action is affordable when it is attempted.
- If an action cannot be afforded, that action fails atomically and is not performed.

A fleet is not itself built as a separate resource-consuming object. It is an operational group assembled from ships that have already been built; the resulting ships travel and fight together.

Ships built during the current player-action phase have a temporary same-turn scrapping exception. Before the player-action phase ends, a player may build and scrap ships built during that same phase without losing the associated resources, including the prototyping cost. This permits experimentation while the phase remains active.

Once the next turn begins, that exception expires. A ship may still be scrapped later, but only for metal and at a heavy loss; the money spent on prototyping and construction is not recovered.

### Resource transfers

Sending resources is an immediate action for both participants:

- A player may send money or metal to another player.
- The transfer is lossless: the recipient receives exactly the amount removed from the sender.
- A player may not send more metal than is currently in the player's metal reserve.
- A player may not send so much money that the player's savings go below the permitted minimum (that is, exceed the permitted debt).
- The sender's and recipient's persistent balances update immediately, in the player's specified action order, and later actions see the updated balances.
- Projected interest and other derived displays may update immediately, but the completed economic phase is not retroactively changed.

Resource transfers are a low-priority feature for implementation.

After an immediate balance change:

- Recalculate and display projected interest as needed.
- Recalculate and display other derived economic values as needed.
- Do not retroactively change the completed economic phase.

### Actions queued for the next turn

Examples include:

- Fleet movements.
- Planet abandonment.
- Diplomatic actions, including offers or alliance changes, if those rules resolve later.

The exact list of immediate versus queued actions remains subject to the rules for each action type.

### Colony abandonment

Abandonment is queued by the player during the player-action phase and occurs at the beginning of the next turn's economic processing, before that turn's income is calculated. Abandoning a colony is free. When a colony is abandoned:

- The colony is removed from the player's owned-colony state.
- The colonists on the planet die.
- Any unmined metal remains on the planet for a future colonizer.
- The player immediately loses the colony's displayed/projected income if it was profitable.
- If the colony had negative income, the player immediately sees the corresponding improvement in projected spending money for future turns.
- Fleets stationed at the planet remain there; abandonment does not remove them.
- Because abandonment occurs before the next economic calculation, the abandoned colony does not contribute income or support costs to that next turn's economic phase.

### Budget changes

The player's fractional budgetary allocations carry forward by default. A player may update them during the player-action phase. The updated allocations apply to the next economic phase, not to the economic phase that has already completed.

## 7. End-turn processing

At the end of the player-action phase:

- Store and log information needed for history and display.
- Store the persistent values needed as inputs to the next turn.
- Preserve immediate savings and metal-reserve changes.
- Preserve queued fleet and player actions for the next applicable phase.
- Recalculate view-only projections as needed for display.
- Persist the complete state needed to resume the game, including pending actions, readiness state, knowledge state, RNG state, and all player, planet, fleet, and technology state.
- Synchronize the critical state needed for host migration at least once per turn.
- Advance the turn and year for the next turn according to their independent rules.
- Reset turn-ready flags.

The completed-turn history record is labeled with the beginning-of-turn snapshot's Turn N/year values. The next canonical snapshot is labeled Turn N+1 with the newly advanced year.

## 8. Readiness and action freezing

The core turn-processing entry point enforces readiness; callers cannot advance the game by bypassing this check.

- A turn begins with all non-eliminated players unready.
- A player may mark ready, become unready, change actions, and mark ready again before processing begins.
- Marking ready freezes neither the player's ability to act nor the game permanently; any later action submitted by that player makes them unready again.
- Immediate actions remain possible after a player has marked ready, but they also make that player unready. Whether the player is automatically re-readied after the action or must explicitly ready again remains an open small policy choice.
- Queued actions are mutable until processing begins and are frozen for the processing pass once the turn advances.
- Eliminated players are excluded from the readiness requirement and have no actions available.
- If a player disconnects, the host must explicitly decide whether to proceed. If the host disconnects, a new host is assigned and inherits that responsibility.
- An AI player becomes ready only after it has completed its planned actions. AI execution should be subject to a configurable turn-length time limit so a loop cannot block the game indefinitely.
- Once all required players are ready and any host-disconnect decision is satisfied, the engine processes the turn atomically and resets readiness for the next turn.

## 9. Outstanding design decisions

The following items remain unresolved and should be settled before this document becomes the final source of truth:

1. Determine the data-driven lower limit for negative savings and the exact bankruptcy rule.
2. Define which diplomatic actions are immediate and which are queued. Diplomacy is intentionally deferred until the available diplomatic actions are better defined; it is not required for the core game flow.
3. Define final ship-design, construction, and scrapping cost formulas, including the data behind the typical prototyping-cost ratio.
4. Define all remaining money and metal flows for combat. Resource-transfer and abandonment rules are specified above; their implementation remains intentionally low priority where noted.
5. Decide whether a player who acts after marking ready is automatically re-readied after the action or must explicitly mark ready again.
6. Define the exact year-increment rule after the starting year 2000.
7. Define the serialized save format and compatibility/versioning policy for complete mid-game resumes.

The following decisions are settled for this draft:

- Immediate-effect player actions are applied in the order specified by the player.
- No action other than the explicitly permitted immediate savings and metal-reserve updates changes the current turn's already-committed economic after-state.
- First-turn starting values are tracked as a player-initialization requirement in `Missing_Implementation_List.md`, rather than as a turn-order decision here.
- Turn N history snapshots represent the beginning of Turn N; Turn 0 begins in year 2000.
- The core engine enforces readiness, excludes eliminated players, requires an explicit host decision after disconnection, and gives AI players a bounded execution window.

Until these decisions are resolved, this document should be treated as an approved direction for discussion rather than a complete implementation specification.
