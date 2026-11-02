# Graduate Mechanics and Balance Analysis

Last Minute Courier | v0.5.1 | November 1, 2026

Required for this student's graduate submission. Based on lecture deck 4, The Art of Game Design: mechanics 1-7 and the balance lenses on slides 104, 112 and 118. The following answers refer to implemented rules. Statements about player feelings are hypotheses until genuine player observation is recorded.

## Seven mechanic categories

| Course mechanic | Implemented model and example | What the player decides |
|---|---|---|
| 1. Space | A continuous, bounded 3D campus with ground, raised landings, ramps, gaps and connected routes. Desks and delivery zones are meaningful subspaces. Height matters: E from beneath a desk is invalid. | Choose a connected route, align with a landing, and reach the correct interaction area. |
| 2. Time | Continuous movement; separate 140/170/200-second order clocks; Q slows active blue-tagged machinery to 0.25 speed for up to three real seconds, followed by four seconds of recharge. Reading is untimed; pause suspends simulation. | Spend the window after observing a gap; release Q after boarding; balance longer safe travel against a deadline. |
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

**6. Does the reward build too quickly or slowly?** The sequence grows from a local coffee response to a populated terrace and then the screening. The agent's full run has 4:07 of successful-order clock time, excluding reading, practice and pause; this is insufficient to establish reward pacing for first-time humans. Record each delivery's wall time and response before changing route length or adding rewards.

## Balance lens 2: Punishment (deck 4, slide 112)

**1. What punishments exist?** Cart contact or falling causes a short setback to a grounded checkpoint. Timeout requires retrying the unfinished order. An explicit retry discards that order's attempt, restores its clock and resets its devices/charge. Earlier parcels and world changes survive. There is no finite life pool, harsh score penalty or full-campaign reset after one error.

**2. Why punish, and what should it achieve?** A visible setback gives traffic timing and safe landings a consequence. Retrying a single order gives the player a chance to apply newly learned timing without replaying already mastered routes. The purpose is attention and learning, not humiliation or lengthening play through repeated work.

**3. Does the punishment feel fair, and why or why not?** The design uses visible machinery, clear failure messages, a brief recovery fade and collision immunity. Native garden play showed that the old checkpoint was too far back and guidance stayed ahead after recovery. v0.5.1 saves the side-ramp landing and rewinds route guidance; engine regression and a fresh input run verified this change. Perceived fairness still needs players' reports; repeated unanticipated recoveries would count against it.

**4. Could a reward produce the same or better effect?** Safe progress is already rewarded by new checkpoints and preserved delivery stamps. The game avoids subtracting delivered stamps or adding an arbitrary time penalty. A cleaner crossing naturally leaves more deadline time, without an extra score system. Removing every setback would make machinery less meaningful; the current compromise keeps the local challenge and limits repeated work.

**5. Are strong penalties matched by strong rewards?** The strongest current setback is replaying one order, not the whole campaign. Its reward is a persistent contribution to the screening, with the final parcel opening the event. Whether the order budgets and repetition are proportionate is a first-time test question. Do not justify a frustrating timeout solely by saying the ending is a reward.

## Balance lens 3: Simplicity and complexity (deck 4, slide 118)

**1. What innate complexity exists?** Players must know the blue-tag rule, three-second allowance, four-second recharge, the difference between reading and accepting, the delivery order and the difference between checkpoint recovery and retry. These rules are explained in cards, practice, contextual hints, the brief and pause help. Seven language options share the same rules.

**2. Can innate complexity become emergent complexity?** One Q rule is used on carts, vertical lifts and a horizontal shuttle. There is no separate freeze power, vehicle control scheme or per-device recharge rule. Observing and combining that one rule with movement creates the later decisions. Directly opening the next untimed brief removes an unnecessary extra interaction.

**3. What emergent interactions arise?** Slowing a cart extends a crossing window but still spends real order time; slowing a lift helps boarding but delays travel if held; a longer ramp avoids precision but costs travel time. Releasing Q can therefore be strategically useful. These interactions are implemented; whether novices independently discover them must be observed.

**4. Is anything too simple?** If players always choose bypasses, they may finish without transferring Q or jumping to a new context. The full agent run verifies those accessible routes, not learning transfer. The next genuine playtest must record route choices and later ability use. If transfer is absent, revise the first lift's instruction/route incentive and retest before claiming the rubric's highest mechanics band. Do not add a hidden gate or unrelated busywork merely to force longer play.

## Evidence and submission relationship

GameScript.md states the complete player-facing rules and modes. ITERATIONS.md and QA-Evidence.json distinguish ordinary-input checks, controlled physics tests and actual human feedback. Human playtests are currently zero. This analysis does not convert a design hypothesis into a tested finding; the final report must add observed player evidence and the corresponding revision commits.
