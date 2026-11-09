# Last Minute Courier — Game Script

Development version v0.7, November 8, 2026. Rules and core mechanics describe implemented behavior. Structure checked against lecture deck 3: slides 19-23 distinguish gameplay modes and shell screens; slide 55 asks for a non-technical rules script. Player timing and response remain measured acceptance tasks.

## Background and goal

A campus film screening is about to begin. The courier carries the three things that make the evening possible: coffee for the audience, a lens for the projector, and the reel for the clocktower screening. A pocket device borrows seconds from marked machinery while the rest of the world keeps moving. The goal is to complete all three deliveries and open the cinema.

## Learning goals

Learn moving and looking; physically jump over a low obstacle; recognize blue clock tags and learn that Q extends a useful gap in machinery movement. Then transfer the same time-control ability to boarding/riding lifts and judging a horizontal shuttle crossing. Alternative arcades and ramps let the player trade route length for precision; golden clocks (+5 s each) sit mainly on the precise lines, so using an ability well is rewarded with time and stars. A practice clock teaches what they are. No new input is introduced during a timed delivery.

## Rules and modes

Three untimed opening cards explain the evening, the ability and the controls (drawn as key caps). A practice yard teaches the three actions. WASD moves, mouse looks, Space jumps, Q slows marked devices, E interacts, C recenters, H opens help, Esc pauses, and R asks whether to restart the current unfinished order.

Walking up to a desk opens its brief (E reopens it after it is closed). The brief lists the parcel, destination, time limit and Q uses; E again or Enter accepts, Esc or Back closes it. Only acceptance starts the order clock. Orders have 60, 150 and 160 seconds and 3, 4 and 5 uses of borrowed time. Each use slows blue devices to 0.25 speed for at most 3 seconds, followed by a 1.5-second recharge; the remaining uses are shown as a count and pips beside the Q key. Practice is unlimited. The courier and order clock do not slow. Pausing freezes gameplay; releasing Q while paused is resolved on resume.

A cart collision or fall returns the courier to a grounded checkpoint with a 0.3-second recovery fade and short collision immunity. The current order remains active. Timeout offers retry and route help. R first asks "Restart this order?" (Enter confirms, Esc keeps going) so a stray key press never discards a run; a confirmed restart reopens that order's brief at its desk. Earlier deliveries and stamps remain. Delivered and final states cannot be undone by R. Play Again starts a fresh run.

## Scene and interaction sequence

1. Read the three cards, select an optional language, and practice in a safe yard with no delivery clock.
2. Accept coffee at the parcel desk. Cross cart traffic or use the arcade; jump or steer around planters. E delivers at the coffee stand and its warm light turns on.
3. Read and accept the lens at the same stand. Turn into the garden, use or bypass the first lift, cross the raised beds and stepping stones (or use the side route), then reach the projection booth via the second lift or ramp. The projector appears after delivery.
4. Read and accept the reel at the booth. Navigate traffic, ride or bypass a horizontal shuttle, ascend to the upper walk and negotiate planters. Follow the central cinema aisle, then turn left toward the roofless clocktower console and its orange square. The screening lights and landscape screen reveal; a cinema ticket shows three stamps and the run statistics.

## Feedback and presentation

Menus are courier slips: a coloured header band, paper body, perforation and key-cap buttons. The in-play HUD uses compact dark plates in the corners: order and destination (top left), the delivery clock that turns red and pulses under 20 seconds plus a rotating compass arrow and distance (top centre), the Q key and charge meter that flashes red when Q is pressed without charge (bottom left), and a single contextual line with the E prompt (bottom centre). Lilita One sets display type and Nunito interface text; Chinese, Japanese and Korean use Noto Sans CJK. Failed actions have explicit messages and recovery. Each completed delivery changes the world, plays a stamp cue and preserves progress. Warm sunset, stationary soft clouds and distant hills support orientation. A 48-second original musical base gains synchronized strings after coffee and bells after the lens; all three layers and interaction sounds share the sound switch. English is the startup language; six optional languages are selected from one menu.

## Comfort and accessibility

Adjust field of view and sensitivity; optionally lock camera direction; use C to aim toward the route. No camera shake, head bob, motion blur or automatic speed zoom. Reading never consumes the order budget. Pausing suspends play, retries preserve earlier work, and route alternatives reduce dependence on precise platform timing. Keyboard menu navigation is supported. Comfort, readability and full mouse interaction still require player verification.

## Iteration and evidence

The user requested a stronger opening, less generic typography, a prettier sky, enjoyable sounds and a comfortable 3D view. Technical checks also found tutorial bypasses, invalid-height interactions, ramp lips, misleading side-route navigation, default-grid terrain and an unbuilt-lighting warning. Implementations and observed regressions are recorded in ITERATIONS.md and QA-Evidence.json. The user reported practice confusion and faint route markings on November 5. Those partial-session observations drove v0.5.3. A complete first-time run and measured 5–10 minute pacing remain required.


## v0.5 moment-to-moment clarity

In practice, cross when the cart moves aside. Q is an optional tool for making the gap last longer. A safe crossing is accepted whether Q was held, released early, or never pressed. The lesson does not reset the courier for choosing natural timing. An actual cart hit still returns to a checkpoint, with an explicit collision message. Crossing saves forward progress.

Continuous orange lines and broad arrows show the current practice or delivery route, including available side paths. A stationary orange ring and downward pointer mark the next waypoint. Future delivery markings are hidden. These visual guides have no collision. Follow the larger direction/distance panel at corners; C gently recenters the view.

At a lift, wait for it to come down without spending Q, then hold Q and walk aboard (Space also works) while it rests at your level. Once aboard, release Q so it carries you normally. At the far landing, walk forward to step off. Contextual prompts follow these stages and remain visible while Q is active. A nearby delivery desk shows ARRIVED and E Deliver package. Reading the next brief has no timer; only Accept delivery starts it.

Coffee delivery places hot cups on the counter and starts the strings. The second delivery brings the audience into the terrace and starts the bells. The last delivery reveals the moonlit screen, three-stamp ticket and wider concluding view. These are implemented feedback; their emotional impact requires player observation.


## Player, perspective and competition

The expected player uses a keyboard and mouse and may be unfamiliar with timing challenges or 3D camera control. The player controls one courier from a following third-person camera, with an avatar-based interaction model. The intended skills are observation, coordinated movement, landing judgment and deciding when to spend a short time-control window. There is one local single-player campaign. There are no opponents, multiplayer rules, leaderboards, difficulty levels or separate time-trial mode. Improving one's own delivery time is an optional personal goal, not another implemented mode.

## Complete mode and transition reference

| Mode or shell screen | Available decisions and actions | Clock and devices | Exit |
|---|---|---|---|
| Opening shell | Three story/control cards; continue/back; language; quit | No delivery timer; devices inactive | Continue after card three enters practice |
| Practice gameplay | Move, look, jump, hold/release Q (unlimited), C, pause/help | No delivery deadline; all carts and lifts run; a striped yard gate stays closed | Actual jump and safe cart-lane crossing with or without Q; reaching the parcel desk opens the first brief |
| Await-pickup gameplay | Walk back to the desk after closing its brief; pause/help | No delivery deadline; carts and lifts keep running; only the marker to the desk is shown | Stepping up to the desk or pressing E there reopens the brief |
| Briefing shell | Read parcel, destination, route and budget; E or Enter accepts; Esc/Back closes | No budget consumed | Accept enters delivery; cancel returns to pickup |
| Delivery gameplay (primary) | Move/look/jump/Q; E delivers; C; help/pause; R asks to restart the current order | Current budget and marked devices advance in real time, with selective Q slowdown | Valid E delivery, timeout, or pause |
| Pause/help shell | Resume; story/route help; sound, camera and language settings; retry when available; quit | Clock, devices, charge and recovery are frozen | Resume returns to the exact preceding gameplay state |
| Delivered shell | View outcome/stamp, 1-3 stars, time left and golden clocks collected; read next brief | Completed delivery fixed; next timer has not started | Read next opens next brief at the same counter |
| Timeout shell | Retry unfinished order; view route help | No ongoing delivery or device movement | Retry reopens that order's brief at its pickup |
| Completion shell | Read ticket, stars per stamp, total out of 9 and rank, delivery time and retries; Play Again | Three deliveries complete; ending animation plays | Play Again returns to opening and resets progress |

Practice and delivery share the same avatar, camera and action vocabulary. Shell screens restrict world actions so reading and settings do not accidentally move the courier. Menu Tab selects controls; Space activates a focused toggle; Enter continues or accepts; sliders use Enter and arrows. A language change does not restart the order; the L shortcut works only on menu screens so it cannot fire mid-delivery. Progress is session-only: quitting does not save a campaign. Play Again preserves selected presentation preferences in the current process; a fresh launch defaults to English.

## Operational rules and edge cases

- Move relative to camera direction. Space jumps from the ground; holding it does not provide flight. Orange markers identify routes/interaction areas; blue clock tags identify devices affected by Q. A physical alternative route does not require a separate unlock.
- Q affects blue carts and platforms, not the courier, scenery or order deadline. Each delivery grants a fixed number of uses (3, 4, 5); starting a slowdown spends one. A use lasts at most three seconds; releasing early ends it and starts a 1.5-second recharge. Pressing Q while recharging or with no uses left does nothing to the world but flashes the meter red, and an empty count also shows a message pointing to gaps and side paths. Retrying an order restores its uses. Holding through exhaustion does not automatically start another use: release and press again.
- On a moving platform, wait for it to reach the near landing, slow it if needed while boarding, release Q to travel at normal speed, and leave at the far landing. Holding Q throughout the ride makes travel slower. The longer side route trades travel time for easier geometry.
- E only reads or delivers within the marked area at a compatible height; standing underneath or far above a desk is invalid. An invalid E does not lose a parcel or advance an order. A reachable destination says ARRIVED and shows the delivery action.
- Coffee, lens and reel must be completed in that order. Accepting gives 60, 150 and 160 seconds respectively (v0.5.4; the earlier 140/170/200 allowed an expert run to finish with over two thirds of every clock unused). Reading a brief never starts its clock. Delivery succeeds when E is used at the destination before the order reaches zero. An expired order must be retried.
- A fall, leaving the playable route or cart contact returns the courier to a safe grounded checkpoint. No stamp or parcel is removed. A brief recovery fade and collision protection prevent an immediate repeated hit. Timers and devices pause during the brief recovery itself; otherwise the order continues. Side-route landings also save checkpoints. Guidance moves back with the courier after recovery.
- Carts, lifts and the shuttle run whenever the courier can move, including practice and waiting for a pickup; collisions count the same everywhere.
- Until the first order is accepted, a striped yard gate after the parcel desk is closed. Walking around it returns the courier to the desk with the message that the gate opens on acceptance.
- E never fails silently: away from a desk or destination it says where to go instead.
- People are solid, and large props stop the camera instead of letting it slide inside them; thin posts and floor markings do not.
- Golden clocks: 4, 5 and 6 on the three routes (one more in practice, which only teaches). Touching one adds 5 seconds to the running order and plays a chime; the clock panel flashes gold. Retrying an order restores its clocks. They never appear outside their own order.
- Stars: at delivery, time left / time limit (capped at 1) of at least 45% earns 3 stars, at least 20% earns 2, any on-time delivery earns 1. The ending totals them out of 9: 9 Legend of the Last Minute, 7-8 Star Courier, 4-6 Reliable Courier, otherwise Courier in Training.
- Feel: holding Q tints the screen cool blue with a vignette and lowers the music's pitch; a cart hit or fall flashes red with a thud; the last 10 seconds tick; deliveries burst into paper confetti.
- R (after confirmation) and the Retry control reset the current unfinished order to its pickup, including its deadline, device positions and Q allowance. Earlier completed orders, stamps, cups, audience and music progression remain. Retry is unavailable during untimed practice and does not undo delivered or completed states.
- The ending's delivery time is the sum of the three successful-order clocks. It excludes story reading, practice, pause, brief recovery and discarded attempts. The Retry counter counts explicit order restarts, not individual falls/cart recoveries. Neither statistic is a complete first-time wall-clock duration.
- There is no life limit, combat, inventory puzzle, hidden score threshold, random failure roll, currency, paid unlock or permanently lost progress within a running session. No special ending requires a fast time.

## Ability teaching and later use

| Ability | Initial demonstrated use | Later available transfer | Validation still needed |
|---|---|---|---|
| Move/look | Reach the practice marker with camera-relative WASD; recenter and mouse controls explained | Choose long arcades or short device routes, turn between landings and align with the final cinema aisle | First-time route comprehension without coaching |
| Jump | Actually jump over the low training step; simply walking around does not pass | Jump across stepping stones and onto planters for golden clocks; step off a raised lift; combine movement and landing judgment | A first-time player must demonstrate a later application; safe routes can bypass these challenges |
| Selective time control | Observe the cart gap; optionally try Q to prolong it. Safe natural timing is accepted | Slow a lift/shuttle for boarding, release it to travel, then judge disembarkation; ride the shuttle over water for its golden clock; ration 5 uses across 6 devices on the last route | Platform physics regression passes; ordinary simultaneous Q + movement input and independent human learning transfer still require testing |

The level provides later applications of every taught ability. Optional bypasses remain for cautious players, so golden clocks and the star rating give a concrete reason to take the applications instead. For the highest rubric band, observe a first-time player recognizing and executing these transfers. If they always bypass them or fail to understand them, change route incentives and instruction placement and test again; do not report the criterion as complete merely because the objects exist.

## Feedback-driven revisions

Ordinary keyboard checks in v0.5.1 led to a shorter coffee awning, open curb/rail junctions, safe side-ramp checkpoints and recovery guidance. v0.5.2 added staged platform instructions but also forced Q use at the training crossing. On November 5 the user reported that waiting for a natural gap was intuitive, yet crossing that way caused respawns; they also found the route unclear. v0.5.3 removes the forced-Q reset, distinguishes real collision recovery, saves crossing progress and adds continuous high-contrast route arrows plus a visible next-waypoint marker. On November 7 a real-speed autoplay that uses only player controls timed out twice at the final shuttle: a side-path guardrail fenced its boarding edge. v0.5.4 keeps rails off every platform footprint and the stepping-stone takeoff, adds a landing lip at the raised final lift, tightens the order budgets to 60/150/160 s after the expert run left most of each clock unused, and aligns two waypoints with painted corners. On November 8 the project owner's first uncoached playthrough of the packaged game found that the desk did not stop them, carts stood still before an order, E gave no response at the counter, and the camera could enter an NPC; they also asked for a visible Q count. v0.6 opens briefs on arrival, adds a yard gate until the first order, runs all devices continuously, explains every E press, gives people and props camera collision, and limits Q to a counted 3/4/5 uses per delivery. The owner then judged it still less fun than commercial games: v0.7 adds golden clocks on the risky lines, a 1-3 star rating per delivery with a total and rank, and audiovisual feel for Q, hits, the final seconds and deliveries.

Three engine regression suites pass, including safe crossings with no Q, early Q release, valid slowdown, real cart contact and route visibility/collision checks. An ordinary-keyboard technical check crossed the practice lane without Q and observed the next lesson with a full charge. These are partial technical checks and actual user-reported practice feedback. No complete human duration, enjoyment, comfort or later learning-transfer result has been recorded. Full observations and commit-linked changes are in ITERATIONS.md, PlaytestFeedback-Nov05.zh.md and QA-Evidence.json.
