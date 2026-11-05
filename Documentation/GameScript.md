# Last Minute Courier — Game Script

Development version v0.5.2, November 4, 2026. Rules and core mechanics describe implemented behavior. Structure checked against lecture deck 3: slides 19-23 distinguish gameplay modes and shell screens; slide 55 asks for a non-technical rules script. Player timing and response remain measured acceptance tasks.

## Background and goal

A campus film screening is about to begin. The courier carries the three things that make the evening possible: coffee for the audience, a lens for the projector, and the reel for the clocktower screening. A pocket device borrows seconds from marked machinery while the rest of the world keeps moving. The goal is to complete all three deliveries and open the cinema.

## Learning goals

Learn moving and looking; physically jump over a low obstacle; recognize blue clock tags and keep Q held while crossing a moving cart lane. Then transfer the same time-control ability to boarding/riding lifts and judging a horizontal shuttle crossing. Alternative arcades and ramps let the player trade route length for precision. No new input is introduced during a timed delivery.

## Rules and modes

Four untimed opening cards explain identity, ability, the delivery plan and controls. A practice yard teaches the three actions. WASD moves, mouse looks, Space jumps, Q slows marked devices, E interacts, C recenters, H opens help, Esc pauses, and R restarts the current unfinished order.

At each desk, E opens a brief. The player reads the parcel, destination and budget, then presses Enter to accept; Back cancels. Only acceptance starts the order clock. Orders have 140, 170 and 200 seconds. Q grants at most 3 seconds at 0.25 device speed, followed by 4 seconds of recharge. The courier and order clock do not slow. Pausing freezes gameplay; releasing Q while paused is resolved on resume.

A cart collision or fall returns the courier to a grounded checkpoint with a 0.3-second recovery fade and short collision immunity. The current order remains active. Timeout offers retry and route help. R restarts only the unfinished order, its devices and ability; earlier deliveries and stamps remain. Delivered and final states cannot be undone by R. Play Again starts a fresh run.

## Scene and interaction sequence

1. Read the four cards, select an optional language, and practice in a safe yard with no delivery clock.
2. Accept coffee at the parcel desk. Cross cart traffic or use the arcade; jump or steer around planters. E delivers at the coffee stand and its warm light turns on.
3. Read and accept the lens at the same stand. Turn into the garden, use or bypass the first lift, cross the raised beds and stepping stones (or use the side route), then reach the projection booth via the second lift or ramp. The projector appears after delivery.
4. Read and accept the reel at the booth. Navigate traffic, ride or bypass a horizontal shuttle, ascend to the upper walk and negotiate planters. Follow the central cinema aisle, then turn left toward the roofless clocktower console and its orange square. The screening lights and landscape screen reveal; a cinema ticket shows three stamps and the run statistics.

## Feedback and presentation

Cream paper panels, ink text, orange route markings and teal clock tags establish distinct roles. The HUD identifies the current parcel, destination, distance/direction, clock and Q charge. Failed actions have explicit messages and recovery. Each completed delivery changes the world, plays a stamp cue and preserves progress. Warm sunset, stationary soft clouds and distant hills support orientation. A 48-second original musical base gains synchronized strings after coffee and bells after the lens; all three layers and interaction sounds share the sound switch. English is the startup language; six optional languages are selected from one menu.

## Comfort and accessibility

Adjust field of view and sensitivity; optionally lock camera direction; use C to aim toward the route. No camera shake, head bob, motion blur or automatic speed zoom. Reading never consumes the order budget. Pausing suspends play, retries preserve earlier work, and route alternatives reduce dependence on precise platform timing. Keyboard menu navigation is supported. Comfort, readability and full mouse interaction still require player verification.

## Iteration and evidence

The user requested a stronger opening, less generic typography, a prettier sky, enjoyable sounds and a comfortable 3D view. Technical checks also found tutorial bypasses, invalid-height interactions, ramp lips, misleading side-route navigation, default-grid terrain and an unbuilt-lighting warning. Implementations and observed regressions are recorded in ITERATIONS.md and QA-Evidence.json. These are development checks; genuine first-time player feedback and measured 5–10 minute pacing remain required.


## v0.5 moment-to-moment clarity

In practice, wait for a gap, then keep Q held while walking from the near edge to the far edge of the blue lane. Release Q after clearing the lane. A short tap followed by an unslowed crossing repeats this untimed lesson from its approach. A completed slow crossing remains valid after releasing Q. During later deliveries, choose when to use the same ability. At a lift, wait for it to come down without spending Q, then hold Q and press Space to jump aboard. Once aboard, release Q so it carries you normally. At the far landing, walk forward to step off. Contextual prompts follow these stages and remain visible while Q is active. A nearby delivery desk shows ARRIVED and E Deliver package. Reading the next brief has no timer; only Accept delivery starts it.

Coffee delivery places hot cups on the counter and starts the strings. The second delivery brings the audience into the terrace and starts the bells. The last delivery reveals the moonlit screen, three-stamp ticket and wider concluding view. These are implemented feedback; their emotional impact requires player observation.


## Player, perspective and competition

The expected player uses a keyboard and mouse and may be unfamiliar with timing challenges or 3D camera control. The player controls one courier from a following third-person camera, with an avatar-based interaction model. The intended skills are observation, coordinated movement, landing judgment and deciding when to spend a short time-control window. There is one local single-player campaign. There are no opponents, multiplayer rules, leaderboards, difficulty levels or separate time-trial mode. Improving one's own delivery time is an optional personal goal, not another implemented mode.

## Complete mode and transition reference

| Mode or shell screen | Available decisions and actions | Clock and devices | Exit |
|---|---|---|---|
| Opening shell | Four story/control cards; continue/back; language; quit | No delivery timer; devices inactive | Continue after card four enters practice |
| Practice gameplay | Move, look, jump, hold/release Q, C, pause/help | No delivery deadline; active training devices | Actual jump and Q held across the training lane, then reach parcel desk |
| Await-pickup gameplay | Walk to current desk; E reads brief; pause/help | No delivery deadline; delivery devices inactive | E at reachable desk enters briefing |
| Briefing shell | Read parcel, destination, route and budget; Enter accepts; Back cancels | No budget consumed | Accept enters delivery; cancel returns to pickup |
| Delivery gameplay (primary) | Move/look/jump/Q; E delivers; C; help/pause; R retries current order | Current budget and marked devices advance in real time, with selective Q slowdown | Valid E delivery, timeout, or pause |
| Pause/help shell | Resume; story/route help; sound, camera and language settings; retry when available; quit | Clock, devices, charge and recovery are frozen | Resume returns to the exact preceding gameplay state |
| Delivered shell | View outcome/stamp; read next brief; language | Completed delivery fixed; next timer has not started | Read next opens next brief at the same counter |
| Timeout shell | Retry unfinished order; view route help; language | No ongoing delivery or device movement | Retry returns to that order's pickup |
| Completion shell | Read ticket and statistics; language; Play Again | Three deliveries complete; ending animation plays | Play Again returns to opening and resets progress |

Practice and delivery share the same avatar, camera and action vocabulary. Shell screens restrict world actions so reading and settings do not accidentally move the courier. Menu Tab selects controls; Space activates a focused toggle; Enter continues or accepts; sliders use Enter and arrows. A language change does not restart the order. Progress is session-only: quitting does not save a campaign. Play Again preserves selected presentation preferences in the current process; a fresh launch defaults to English.

## Operational rules and edge cases

- Move relative to camera direction. Space jumps from the ground; holding it does not provide flight. Orange markers identify routes/interaction areas; blue clock tags identify devices affected by Q. A physical alternative route does not require a separate unlock.
- Q affects active blue carts and platforms, not the courier, scenery or order deadline. A full use has a maximum three-second allowance. Releasing early ends the effect and still starts a four-second recharge; unused allowance is not banked across recharge. At the end of recharge the full allowance returns. Holding through exhaustion does not automatically start another use: release and press again.
- On a moving platform, wait for it to reach the near landing, slow it if needed while boarding, release Q to travel at normal speed, and leave at the far landing. Holding Q throughout the ride makes travel slower. The longer side route trades travel time for easier geometry.
- E only reads or delivers within the marked area at a compatible height; standing underneath or far above a desk is invalid. An invalid E does not lose a parcel or advance an order. A reachable destination says ARRIVED and shows the delivery action.
- Coffee, lens and reel must be completed in that order. Accepting gives 140, 170 and 200 seconds respectively. Reading a brief never starts its clock. Delivery succeeds when E is used at the destination before the order reaches zero. An expired order must be retried.
- A fall, leaving the playable route or cart contact returns the courier to a safe grounded checkpoint. No stamp or parcel is removed. A brief recovery fade and collision protection prevent an immediate repeated hit. Timers and devices pause during the brief recovery itself; otherwise the order continues. Side-route landings also save checkpoints. Guidance moves back with the courier after recovery.
- R and the Retry control reset the current unfinished order to its pickup, including its deadline, device positions and Q allowance. Earlier completed orders, stamps, cups, audience and music progression remain. Retry is unavailable during untimed practice and does not undo delivered or completed states.
- The ending's delivery time is the sum of the three successful-order clocks. It excludes story reading, practice, pause, brief recovery and discarded attempts. The Retry counter counts explicit order restarts, not individual falls/cart recoveries. Neither statistic is a complete first-time wall-clock duration.
- There is no life limit, combat, inventory puzzle, hidden score threshold, random failure roll, currency, paid unlock or permanently lost progress within a running session. No special ending requires a fast time.

## Ability teaching and later use

| Ability | Initial demonstrated use | Later available transfer | Validation still needed |
|---|---|---|---|
| Move/look | Reach the practice marker with camera-relative WASD; recenter and mouse controls explained | Choose long arcades or short device routes, turn between landings and align with the final cinema aisle | First-time route comprehension without coaching |
| Jump | Actually jump over the low training step; simply walking around does not pass | Jump across stepping stones and onto or off a changing-height service platform; combine movement and landing judgment | A first-time player must demonstrate a later application; safe routes can bypass these challenges |
| Selective time control | Keep Q held from the near edge to the far edge of the blue training lane | Slow a lift/shuttle for boarding, release it to travel, then judge disembarkation; combine timing with movement and jumping | Platform physics regression passes; ordinary simultaneous Q + movement input and independent human learning transfer still require testing |

The level provides later applications of every taught ability, but the optional bypasses mean completion alone does not prove all applications were used. For the highest rubric band, observe a first-time player recognizing and executing these transfers. If they always bypass them or fail to understand them, change route incentives and instruction placement and test again; do not report the criterion as complete merely because the objects exist.

## Feedback-driven changes in v0.5.1

During ordinary keyboard play, the coffee awning crossed the camera on exit, decorative continuous curbs resembled roadblocks at merges, and garden recovery sent the courier back beyond the side ramp while guidance stayed ahead. The awning was shortened, curb/rail spans now leave supported intersections open, and safe side landings save progress while recovery rewinds guidance. A fresh ordinary-input run then completed all three orders without changing player position or state through a fixture. It included recoveries and pauses, optional camera lock, and read-only position/time logging; it is an agent technical run, not first-time human feedback.

A post-run German check exposed awkward stamp-word wrapping. Stamp spacing/type and separate delivery-time/retry lines were adjusted. Pause transitions now clear held-key state and pending movement so the courier does not resume stale input. Detailed evidence and local commits are indexed in ITERATIONS.md and QA-Evidence.json. Changes must be pushed to the required repository before submission; a local history or bundle is not a GitHub submission.


## Feedback-driven changes in v0.5.2

Review of the standalone practice found that tapping Q briefly before walking through at full device speed still passed. Practice now checks Q staying active across the lane, with explicit release and recharge guidance. Lift hints explain waiting for a low platform, using Space to board, releasing Q during travel, and walking off at the landing. Active slowdown no longer masks the boarding prompt. Three engine regression suites pass with zero test errors or warnings, including early release, side bypass, recovery and context transitions. These controlled checks do not replace normal simultaneous keyboard input or a first-time human playtest.
