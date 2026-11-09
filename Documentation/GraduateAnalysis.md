# Last Minute Courier - Graduate Analysis

Yu Han | Project 3 | v0.10.1 | November 8, 2026

Mechanic categories and the Reward, Punishment, and Simplicity/Complexity lenses follow lecture 4, The Art of Game Design. Answers describe implemented design choices and aggregate player feedback.

## Graduate analysis: seven mechanic categories

| Category | Implementation and player decision |
|---|---|
| Space | A bounded, continuous 3D campus with paths, raised landings, water, gaps and delivery areas. Height and connectivity matter: the player must stand at the destination, not underneath it. Routes trade travel distance against landing precision. |
| Time | Continuous movement, periodic machinery and separate delivery clocks. Q slows device time while real deadline time continues. Waiting, extending a gap and releasing Q after boarding have different costs. |
| Objects, attributes and states | The courier has position and movement states; parcels have accepted/delivered/failed states; the lens has condition; the Borrower has charge and uses. Devices have position or phase. Collected rewards disappear, and delivery stamps and world changes persist within the run. |
| Actions | Basic verbs are move, look, sprint, jump/climb, slide, hold/release Q and interact. Strategic actions combine them: slow a shutter and slide, jump a turnstile arm, release Q after boarding, or climb toward a reward line. |
| Rules | Orders must be delivered in sequence within 45/120/130 s. Q has 3/4/5 initial uses and pickups replenish it. Hits, checkpoints, lens damage, stars and retries define consequences. Three deliveries complete the campaign. |
| Skill | Physical skill coordinates movement, camera, jumping and sliding. Mental skill observes cycles, estimates windows and compares routes. Learning improves the player's timing rather than raising an avatar statistic. |
| Chance | Gameplay-critical motion is periodic rather than decided by random success rolls. An unfamiliar arrival phase creates uncertainty, but the player can observe and learn it. Cosmetic variation does not determine delivery success. |

The categories describe different parts of the same interaction. Boarding a lift combines 3D space, changing object attributes, a temporary time effect, player action, limited-use rules and timing skill. A successful landing is not awarded by a random roll.

## How the Time theme changes play

The Borrower grants a local opportunity instead of pausing the whole game. Slowing traffic can make a gap easier to cross, but slowing a lift for too long also delays its arrival. Releasing Q is therefore a meaningful action. A fixed per-order allowance prevents holding the power continuously, while hourglasses and spark trails create a choice between safety now and more options later.

The parcels give the deadline different meanings. Coffee loses warmth, the lens demands care, and the reel must reach the screening. These concerns lead to different star rules while retaining one consistent movement and time-control system.

## Balance lens: Reward

**1. What rewards exist, and what other forms are possible?** Deliveries give stamps, confetti, a sound cue and a persistent contribution to film night. Hourglasses grant one Q use; 25 sparks grant another; close calls grant sparks. Parcel-specific stars combine into the final ticket's rank. Cosmetic unlocks are possible, but are unnecessary for this short campaign.

**2. Are rewards exciting or boring, and why?** The intended appeal is both immediate feedback and a larger shared goal: a cup-filled stand, a working projector, an audience and finally the film. Player feedback described the earlier experience as monotonous and too easy. More expressive traversal, spark trails and parcel-specific scoring respond to that concern rather than simply increasing collectible counts.

**3. Do players understand the reward?** The earlier blue pickup looked dangerous, and players avoided it. The revision makes it an attracting gold hourglass, distinct from blue machinery, with a pickup cue and an explicit extra-use message. Destination stamps and visible world changes connect a delivery to its outcome; three stamps become the admission ticket.

**4. Is reward timing too regular, and should it vary?** Three major delivery rewards are predictable because the story has three concrete needs. Smaller sparks, hourglasses and close-call bonuses vary with movement and route risk between them. Random schedules would weaken the link between a successful action and its result.

**5. How are the rewards connected?** Coffee supplies the audience, the lens enables projection and the reel provides the film. The musical arrangement accumulates with those contributions. The ticket reuses the same delivery stamps, so the ending belongs to the player's work rather than appearing as an unrelated prize.

**6. Does the reward build too quickly or slowly?** The intended progression moves from a small local response to a populated terrace and a complete screening. Optional stars and riskier spark lines provide a reason to improve a finished run. Reward pacing is adjusted alongside movement speed and route difficulty; compulsory waiting or repeated text is not used to lengthen play.

## Implementation consequences

Rewards support the same action vocabulary rather than introducing a separate economy. A harder line can replenish the resource used to attempt it. Stars recognize the customer's concern, while delivered stamps are preserved even after a later retry. The player can finish the story with a modest score and then replay for a cleaner ticket.

## Balance lens: Punishment

**1. What punishments exist?** Moving-obstacle contact causes knock-back and a brief stagger; a fall returns to a checkpoint. During the lens order, hits and falls damage the case and the third fails that attempt. A deadline failure requires retrying only the unfinished order. There is no finite life pool or loss of earlier delivery stamps.

**2. Why punish, and what should it achieve?** A setback gives traffic timing and safe landings a consequence. A local retry lets the player apply the timing just learned without repeating completed deliveries. Its purpose is attention and improvement rather than humiliation or artificial length.

**3. Does the punishment feel fair, and why?** Earlier practice feedback identified an unfair rule in earlier practice: crossing safely after the cart passed still caused a reset because Q was required. That reset was removed. Later, overly fast turnstiles and unclear key response led to slower rotations, gentler collision margins, the C slide key and lit key caps. The design now ties setbacks to visible contact or falling, with a written reason.

**4. Could a reward produce the same or a better effect?** Clean crossings already preserve deadline time, lens condition and stars. Checkpoints and persistent stamps reward progress. The game therefore avoids additional arbitrary time penalties or subtracting earlier stamps; it keeps enough consequence to make machinery matter.

**5. Are strong penalties matched by strong rewards?** The largest ordinary setback is replaying one order. Its reward is a permanent contribution to the screening, culminating in the film and ticket. Parcel budgets and checkpoints limit repeated work, while the stronger lens condition rule gives careful play a distinct purpose. A full campaign reset would be disproportionate for this audience.

## Recovery is distinct from retry

A fall is a local recovery while the active clock continues. A timeout or broken lens ends the attempt and offers a fresh brief. A requested restart first shows a confirmation. These states have different consequences and are described separately in the Game Script and player-facing messages.

## Balance lens: Simplicity and complexity

**1. What innate complexity exists?** Players must understand the blue-tag rule, Q duration and recharge, the per-order use count, reading versus accepting, parcel order and recovery versus retry. Cards introduce the controls; practice demonstrates movement; briefs show the parcel's budget and allowance; contextual hints explain the current obstacle.

**2. Can innate complexity become emergent complexity?** The same Q rule affects carts, lifts, the shuttle, shutters and turnstiles. It combines with existing movement instead of adding a separate control system for each device. A later route can therefore ask a new question - when to release Q after boarding, or how to fit a slide under a slowing door - without adding another power.

**3. What emergent interactions arise?** Slowing a cart extends a crossing window but still consumes deadline time. Slowing a lift helps boarding but delays travel if held too long. A longer ramp reduces precision demands at the cost of time. A harder spark line can replenish the use spent reaching it. Observation and route choice produce these tradeoffs from a small rule set.

**4. Is anything too simple?** Earlier feedback said the game was too easy and monotonous. Parkour combinations and compulsory turnstile gates add active decisions. Optional ramps and bypasses remain to support novices. Removing the forced-Q practice reset avoids mistaking an arbitrary requirement for useful complexity; slowing the turnstiles avoids confusing difficulty with unreadable motion.
