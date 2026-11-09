# Last Minute Courier - Game Script

Yu Han | Project 3 | v0.10.1 | November 8, 2026

## Background, player and goal

A campus film night needs coffee, a projector lens and its final reel. One courier completes these deliveries in sequence, using a third-person camera and an avatar-based interaction model. The audience understands keyboard and mouse controls but may be new to 3D timing games. This script describes the implemented single-player campaign; there is no multiplayer or separate time-trial mode.

## Teaching and progression

The opening establishes who the player is, why the parcels matter, and how to use the Borrower. The practice yard then teaches movement without a delivery deadline. Only accepting an order starts its clock; reading a brief is always untimed.

| Ability | Introduction | Later application |
|---|---|---|
| Move, look and sprint | Key-cap card, safe yard and sprint prompt. | Choose a longer safe path or a faster line; sprint to a departing platform or into the springboard. |
| Jump and climb | Physically jump the practice step; the first crate wall explains jumping into a ledge to climb it. | Cross stepping stones, clear a low turnstile arm, climb crate stairs to the market roof. |
| Slide | C or Ctrl slides under a practice beam spanning the yard. | Pass a closing shutter or the high turnstile arm; combine slide timing with Q. |
| Borrow time | Blue tags and the Q meter explain a temporary slowdown; practice offers a cart-lane example. A safe crossing also works without Q. | Slow a lift or shuttle to board, then release Q for travel; slow a shutter or turnstile to combine movement with a readable gap. |
| Interact | Reaching the desk opens its brief; E or Enter accepts. | Deliver at the marked destination, then read and accept the next parcel. |

Teaching progresses from isolated actions to combinations. The first crate climb is introduced on the coffee route before later roof use. Some platform and climb challenges have longer alternatives; the three turnstile gates are compulsory.

## Complete player sequence

**Opening and practice.** Read three cards: the film-night story, borrowed time, and controls. Practice sprinting, jumping and sliding; observe the cart and cross after it passes, optionally extending the gap with Q. The route leads to the parcel desk. The yard exit stays closed until the first parcel is accepted.

**Coffee.** Accept the coffee, cross the traffic or take the arcade, and meet the first crate wall and rolling shutter. Clear the turnstile gate and deliver at the coffee stand. Coffee warmth determines stars. Cups, steam, lights and the strings layer make the first contribution visible and audible.

**Lens.** Accept the lens at the coffee stand. Navigate the garden's lift or ramp, raised beds and stepping stones or side walk; combine Q with the turnstile's jump/slide pattern. Reach the projection booth by lift or ramp and pass its shutter. Hits and falls damage the case. Delivery prepares the projector, brings the audience and adds bells.

**Reel.** Accept the reel at the booth. Use traffic gaps or the market roof, pass the market shutter, ride or bypass the shuttle, clear the final turnstile and reach the upper walk by lift or springboard. Follow the central cinema aisle to the marked console. Delivery reveals the screening and a ticket showing three stamps, stars and the run's rank.

## Rules and all modes

| Rule | Implemented behavior |
|---|---|
| Controls | WASD move; mouse look; hold Shift sprint; Space jump/climb; C or Ctrl slide; hold Q borrow time; E interact; V recenter; H help; Esc pause; R request restart. L changes language on menu screens. |
| Delivery clocks | Coffee 45 s; lens 120 s; reel 130 s. Acceptance starts the current clock. The courier must reach the destination and interact before it expires. |
| Borrower | Blue carts, lifts, shuttle, shutters and turnstiles run at 0.25 speed while Q is held, for at most 3 real seconds per use. The courier and deadline stay at normal speed. Release Q to end early; the use is spent and recharge takes 1.5 s. |
| Uses and pickups | Each order starts with 3/4/5 uses; practice is unlimited. Each gold hourglass gives one extra use. Every 25 gold sparks gives one use. Close calls add sparks. Practice has one hourglass; deliveries have 4/5/7. |
| Traffic and falls | Cart or moving-obstacle hits knock the courier back with a short stagger. Falls return to a grounded checkpoint with an explanation. Neither automatically discards the current parcel. |
| Lens condition | Every hit or fall damages the case. Its third knock fails the lens order. Retry resets that attempt's case, clock, devices and uses. |
| Stars | Coffee and reel: at least 45% clock left earns 3 stars, at least 20% earns 2, otherwise 1. Lens: undamaged earns 3, one knock 2, two knocks 1. Total is out of 9. |
| Progress and restart | Earlier deliveries and their world changes survive retrying an unfinished order. R first asks for confirmation. Play Again starts a new run. There is no saved campaign across quitting. |

## Gameplay modes and shell screens

| State | Actions, time and transitions |
|---|---|
| Opening cards | Read story, ability and controls; navigate cards or choose language. No delivery clock. Continue enters practice. |
| Untimed practice / collection | Move and learn in the yard or approach a desk. Reaching the desk opens the brief; E can reopen a closed brief. |
| Brief | Read parcel, destination, clock and Q uses. E/Enter accepts; Back/Esc closes. No delivery clock until accepted. |
| Timed delivery | Traverse, collect rewards and deliver. Only this state consumes the order budget. E at the destination completes the order. |
| Pause / help / settings | Read controls; change sound, camera or language; resume. Simulation and delivery clock are suspended. |
| Restart confirmation | Confirm restarting only the unfinished order or cancel and keep it. The clock is suspended during the decision. |
| Delivery result | Review stars and parcel outcome; continue to the next untimed brief. Completed contributions persist. |
| Timeout / damaged lens | Read the reason, view route help or retry. Retry returns to that parcel's brief rather than resetting the campaign. |
| Completion | View the screening and ticket; Play Again returns to the opening or quit. No further deadline. |

The local single-player campaign has practice and timed delivery gameplay. Cards, briefs, help and results are shell screens that organize reading, decisions and transitions.

## Operational rules and edge cases

- Move relative to camera direction. Space jumps from the ground; holding it does not provide flight. Orange markers identify routes/interaction areas; blue clock tags identify devices affected by Q. A physical alternative route does not require a separate unlock.
- Q affects blue carts, lifts, the shuttle, rolling shutters and turnstiles, not the courier or order deadline. Each delivery starts with a fixed number of uses (3, 4, 5), and each gold hourglass, and every 25 sparks, picked up on the way adds one; starting a slowdown spends one. A use lasts at most three seconds; releasing early ends it and starts a 1.5-second recharge. Pressing Q while recharging or with no uses left does nothing to the world but flashes the meter red, and an empty count also shows a message pointing to gaps and side paths. Retrying an order restores its uses. Holding through exhaustion does not automatically start another use: release and press again.
- On a moving platform, wait for it to reach the near landing, slow it if needed while boarding, release Q to travel at normal speed, and leave at the far landing. Holding Q throughout the ride makes travel slower. The longer side route trades travel time for easier geometry.
- E only reads or delivers within the marked area at a compatible height; standing underneath or far above a desk is invalid. An invalid E does not lose a parcel or advance an order. A reachable destination says ARRIVED and shows the delivery action.
- Coffee, lens and reel must be completed in that order. Accepting gives 45, 120 and 130 seconds respectively. Reading a brief never starts its clock. Delivery succeeds when E is used at the destination before the order reaches zero. An expired order must be retried.
- A fall into water, off a raised walkway or out of the delivery's area returns the courier to a safe grounded checkpoint with a message naming the cause. Cart contact is a knock-back with a 0.7 s stagger and brief immunity, not a teleport. No stamp or parcel is removed.
- Carts, lifts and the shuttle run whenever the courier can move, including practice and waiting for a pickup; collisions count the same everywhere.
- Until the first order is accepted, a striped yard gate after the parcel desk is closed. Walking around it returns the courier to the desk with the message that the gate opens on acceptance.
- E never fails silently: away from a desk or destination it says where to go instead.
- People are solid, and large props stop the camera instead of letting it slide inside them; thin posts and floor markings do not.
- Carts are traffic: each lane has a depot shed at both ends and carts drive out of one, across the lane and into the other, one way. Q slows the cart's clock, so it waits in its depot longer.
- Gold hourglasses: 4, 5 and 7 on the three routes (one more in practice, which only teaches). They drift toward a courier who comes within about 3 m; touching one gives Q one more use in the running order, pops "+1 borrowed time" and flashes the Q key. Most sit on the harder line. Retrying an order restores them.
- Gold sparks are laid every 2.5 m along each recommended line, along the market roof and along the springboard's flight. They drift in from about 2.4 m; each run of pickups raises the chime's pitch. Every 25 sparks give one more Q use (a small gold bar under the Q pips shows progress). A close call adds sparks: sliding through a shutter while it is below 165 cm (+3), clearing a turnstile arm (+2), crossing a lane within 3 m of a moving cart without touching it (+2). Retrying an order restores its sparks and empties the bar.
- Parkour: Shift raises run speed from 5.2 to 8.2 m/s. C or Ctrl starts a 0.75 s slide (pressed in the air, it fires on landing) (from 10.5 m/s easing to 3.8 m/s) that halves the courier's height to 88 cm; the courier stands again as soon as there is headroom. A jump that meets a ledge 35-230 cm above the feet on a walkable surface (crates, decks, planters; never hedges, sheds or roofs that leave the level) climbs it in about a third of a second. A jump pressed up to 0.14 s after running off an edge still works, and one pressed up to 0.16 s before landing fires on touchdown.
- Rolling shutters (blue, clock-tagged, Q slows them): open 40% of each cycle, roll down over 22%, stay shut 18%, roll up 20%. A shut door is solid and harmless; a door coming down on a standing courier knocks them back (and knocks the lens case). A slide fits while the door is above about 95 cm.
- Turnstiles (blue, Q slows them): two arms on one hub, a low arm (35-75 cm) to jump and a high arm (120-160 cm) to slide under. One turn takes 10, 8.5 and 7.5 s on the three routes. Each stands in a wall from hedge to hedge, like a ticket gate, so every route passes through it. Being swept is a knock-back like a cart (and knocks the lens case). Springboards (orange, not affected by Q) launch a courier who runs across them about 5.6 m up, carrying their run speed.
- Stars, judged by each customer's concern: coffee by warmth (time left / limit: at least 45% earns 3, at least 20% earns 2, otherwise 1); the lens by care (no knocks 3, one knock 2, two knocks 1; a third knock cracks it); the reel by time left before the screening (same thresholds as coffee). The ending totals them out of 9: 9 Legend of the Last Minute, 7-8 Star Courier, 4-6 Reliable Courier, otherwise Courier in Training.
- Feel: holding Q tints the screen cool blue with a vignette and lowers the music's pitch; a cart hit or fall flashes red with a thud; the last 10 seconds tick; deliveries burst into paper confetti.
- R (after confirmation) and the Retry control reset the current unfinished order to its pickup, including its deadline, device positions and Q allowance. Earlier completed orders, stamps, cups, audience and music progression remain. Retry is unavailable during untimed practice and does not undo delivered or completed states.
- The ending's delivery time is the sum of the three successful-order clocks. The Retry counter counts explicit restarts.



## Presentation, comfort and feedback

Courier-slip menus and a compact corner HUD separate extended reading from moment-to-moment play. A continuous orange recommended route, next-waypoint ring and moving-platform chevrons identify progress. Blue tags identify Q devices; gold identifies rewards. Only the current task's route is emphasized. English is the startup language; a menu offers Chinese, Japanese, Korean, Spanish, French and German. A fresh launch defaults to English.

The camera has adjustable sensitivity and field of view, an optional direction lock and V recentering. There is no head bob, shake or motion blur; sprinting and sliding gently widen the view. The original 48-second score adds synchronized strings after coffee and bells after the lens. The sound switch controls music and effects. Reading and settings never consume the delivery budget.

## Feedback-driven revisions

Feedback from four v0.9/v0.10 players informed the latest revisions. Earlier practice confusion led to accepting natural cart gaps without Q and clearer routes. Collection confusion led to automatic desk briefs and an acceptance gate. Blue rewards that looked unsafe became attracting gold hourglasses; monotonous/easy play motivated parkour and combined devices. Fast turnstiles and uncertain inputs led to slower compulsory gates, C slide, V recenter and lit key caps. The v0.10.1 camera fix prevents the projection-booth shutter from filling the view. ITERATIONS.md and PlaytestFeedback-Nov08.md retain the revision trace.
