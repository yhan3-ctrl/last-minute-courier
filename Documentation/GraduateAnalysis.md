# Graduate Mechanics and Balance Analysis

Last Minute Courier | v0.5.4 | November 7, 2026

Required for this student's graduate submission. Based on lecture deck 4, The Art of Game Design: mechanics 1-7 and the balance lenses on slides 104, 112 and 118. The following answers refer to implemented rules. November 5 practice feedback is recorded explicitly; other player-feeling claims remain hypotheses.

## Seven mechanic categories

| Course mechanic | Implemented model and example | What the player decides |
|---|---|---|
| 1. Space | A continuous, bounded 3D campus with ground, raised landings, ramps, gaps and connected routes. Desks and delivery zones are meaningful subspaces. Height matters: E from beneath a desk is invalid. | Choose a connected route, align with a landing, and reach the correct interaction area. |
| 2. Time | Continuous movement; separate 60/150/160-second order clocks; Q slows active blue-tagged machinery to 0.25 speed for up to three real seconds, followed by four seconds of recharge. Reading is untimed; pause suspends simulation. | Spend the window after observing a gap; release Q after boarding; balance longer safe travel against a deadline. |
| 3. Objects, attributes and states | Courier: position, grounded/jumping/recovering, held parcel. Cart: lane position and time phase. Platform: location and boarding/riding state. Order: unread, accepted, delivered or expired. Borrower: ready, active or recharging. Desks, stamps and the cinema have persistent delivered states within a run. | Read visible cart/platform motion, charge bar, delivery count and clock to choose the next action. |
| 4. Actions | Basic actions: move, look, jump, hold/release Q and interact. Strategic actions: wait for a gap, preserve a charge for boarding, release it during travel, choose a bypass, or retry an order after learning its route. | Combine a small set of verbs in different sequences. Settings and help are shell actions, not additional courier powers. |
| 5. Rules | Three ordered parcels, reachable-height interaction, limited Q use, grounded checkpoints, order-specific timeouts and retries, and a three-delivery completion condition. The game enforces these rules; the script states their consequences. | Complete the current goal within its budget, recover from a mistake, and understand which progress persists. |
| 6. Skill | Physical: coordinate camera-relative movement, jumping and landings. Mental: observe periodic machinery, estimate a safe interval and compare routes. No social opponent or avatar-stat upgrade system is required. | Improve real observation and coordination through practice rather than buying numerical power. |
| 7. Chance | Gameplay-critical traffic and platform movement are periodic, not random success rolls. Arrival timing can feel uncertain to a novice; motion can be observed and learned. Cosmetic variation does not decide whether a delivery succeeds. | Take an informed timing risk or choose a longer route; failure is not assigned by a dice roll. |

These categories describe different aspects of the same interaction. For example, slowing a lift combines time, a platform's changing attributes, player action, limited-charge rules, 3D landing space and player skill. Its success is not a random probability check.

## Balance lens 1: Reward (deck 4, slide 104)

**1. What rewards exist, and what other forms are possible?** Each successful delivery gives a visible stamp, an explicit delivery result, a short sound and a persistent world change. Coffee adds cups/steam and strings; the lens prepares the projector, audience and bells; the reel reveals the screen and combines the stamps into a cinema ticket. These are implemented. A numerical score or cosmetic shop is not implemented and is not needed for the current cooperative story.

**2. Are rewards exciting or boring, and why?** The design hypothesis is that an inhabited scene and a seat at the event make the task feel consequential. An agent can verify that objects, result text and music-layer state change; it cannot establish excitement. Ask first-time players what changed after each delivery and which moment they cared about, recording contrary responses as well.

**3. Do players understand the reward?** A completion count, named parcel stamp, written outcome and changed destination communicate the same event through several channels. The final card explains that three stamps form one admission ticket. Comprehension remains a playtest criterion: the tester should explain why the ticket appears without coaching.

**4. Is the reward too regular, and should timing vary?** Three major rewards are deliberately predictable because there are exactly three concrete deliveries. Their intervals and presentation differ with the route and parcel. Random reward schedules would obscure the causal link between helping and the scene changing. Whether a route's interval feels too long must be measured, not assumed.

**5. How are rewards connected?** They are preparation for one film night: audience comfort, projection capability and film content. The same delivery stamps become the final ticket, and musical layers accumulate rather than replacing unrelated tracks. This connection is part of both the implemented world and the opening story.

**6. Does the reward build too quickly or slowly?** The sequence grows from a local coffee response to a populated terrace and then the screening. The Nov 7 real-speed autoplay (ordinary player verbs, no teleporting) reached the ending in 3:00 wall time using 12.3 / 62.8 / 63.0 s of order clock. It knows every crossing in advance, so this is an expert lower bound and is insufficient to establish reward pacing for first-time humans. Record each delivery's wall time and response before changing route length or adding rewards.

## Balance lens 2: Punishment (deck 4, slide 112)

**1. What punishments exist?** Cart contact or falling causes a short setback to a grounded checkpoint. Timeout requires retrying the unfinished order. An explicit retry discards that order's attempt, restores its clock and resets its devices/charge. Earlier parcels and world changes survive. There is no finite life pool, harsh score penalty or full-campaign reset after one error.

**2. Why punish, and what should it achieve?** A visible setback gives traffic timing and safe landings a consequence. Retrying a single order gives the player a chance to apply newly learned timing without replaying already mastered routes. The purpose is attention and learning, not humiliation or lengthening play through repeated work.

**3. Does the punishment feel fair, and why or why not?** The November 5 user reported that their intuitive strategy was to wait for the cart to move aside and cross quickly, but the forced-Q lesson respawned them. That was a mismatch between the visible danger and the actual failure rule. v0.5.3 accepts safe timing with or without Q and names an actual cart hit in its recovery message. Earlier side-ramp checkpoints still limit lost progress. Engine checks cover both safe passage and real contact; the player's response to the new build is not yet known.

**4. Could a reward produce the same or better effect?** Safe progress is already rewarded by new checkpoints and preserved delivery stamps. The game avoids subtracting delivered stamps or adding an arbitrary time penalty. A cleaner crossing naturally leaves more deadline time, without an extra score system. Removing every setback would make machinery less meaningful; the current compromise keeps the local challenge and limits repeated work.

**5. Are strong penalties matched by strong rewards?** The strongest current setback is replaying one order, not the whole campaign. Its reward is a persistent contribution to the screening, with the final parcel opening the event. Budgets were tightened in v0.5.4 from 140/170/200 to 60/150/160 s, about 2.5-5x the expert autoplay time, so the deadline matters without demanding mastery. Whether the budgets and repetition are proportionate for a first-time player is still a test question. Do not justify a frustrating timeout solely by saying the ending is a reward.

## Balance lens 3: Simplicity and complexity (deck 4, slide 118)

**1. What innate complexity exists?** Players must know the blue-tag rule, three-second allowance, four-second recharge, the difference between reading and accepting, the delivery order and the difference between checkpoint recovery and retry. These rules are explained in cards, practice, contextual hints, the brief and pause help. Seven language options share the same rules.

**2. Can innate complexity become emergent complexity?** One Q rule is used on carts, vertical lifts and a horizontal shuttle. There is no separate freeze power, vehicle control scheme or per-device recharge rule. Observing and combining that one rule with movement creates the later decisions. Directly opening the next untimed brief removes an unnecessary extra interaction.

**3. What emergent interactions arise?** Slowing a cart extends a crossing window but still spends real order time; slowing a lift helps boarding but delays travel if held; a longer ramp avoids precision but costs travel time. Releasing Q can therefore be strategically useful. These interactions are implemented; whether novices independently discover them must be observed.

**4. Is anything too simple?** Optional bypasses can still let a player finish without transferring Q or jumping to a later device. Forcing Q during an otherwise safe crossing caused documented user confusion, so v0.5.3 teaches it as a useful choice and retains explicit lift boarding/release guidance. The user also found the route unclear; larger arrows, current-order highlighting and a visible next waypoint reduce navigation ambiguity. Later independent ability use and the clarity of the revised route still require observation.

## Evidence and submission relationship

GameScript.md states the complete player-facing rules and modes. ITERATIONS.md and QA-Evidence.json distinguish ordinary-input checks, controlled physics tests and actual human feedback. Human playtests are currently zero. This analysis does not convert a design hypothesis into a tested finding; the final report must add observed player evidence and the corresponding revision commits.
