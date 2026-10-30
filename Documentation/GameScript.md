# Last Minute Courier — Game Script

Development version v0.4. This describes implemented behavior. Lecture-specific formatting remains to be checked; pacing and player response are not measured claims.

## Background and goal

A campus film screening is about to begin. The courier carries the three things that make the evening possible: coffee for the audience, a lens for the projector, and the reel for the clocktower screening. A pocket device borrows seconds from marked machinery while the rest of the world keeps moving. The goal is to complete all three deliveries and open the cinema.

## Learning goals

Learn moving and looking; physically jump over a low obstacle; recognize blue clock tags and hold Q near a moving cart. Then transfer the same time-control ability to boarding/riding lifts and judging a horizontal shuttle crossing. Alternative arcades and ramps let the player trade route length for precision. No new input is introduced during a timed delivery.

## Rules and modes

Four untimed opening cards explain identity, ability, the delivery plan and controls. A practice yard teaches the three actions. WASD moves, mouse looks, Space jumps, Q slows marked devices, E interacts, C recenters, H opens help, Esc pauses, and R restarts the current unfinished order.

At each desk, E opens a brief. The player reads the parcel, destination and budget, then presses Enter to accept; Back cancels. Only acceptance starts the order clock. Orders have 140, 170 and 200 seconds. Q grants at most 3 seconds at 0.25 device speed, followed by 4 seconds of recharge. The courier and order clock do not slow. Pausing freezes gameplay; releasing Q while paused is resolved on resume.

A cart collision or fall returns the courier to a grounded checkpoint with a 0.3-second recovery fade and short collision immunity. The current order remains active. Timeout offers retry and route help. R restarts only the unfinished order, its devices and ability; earlier deliveries and stamps remain. Delivered and final states cannot be undone by R. Play Again starts a fresh run.

## Scene and interaction sequence

1. Read the four cards, select an optional language, and practice in a safe yard with no delivery clock.
2. Accept coffee at the parcel desk. Cross cart traffic or use the arcade; jump or steer around planters. E delivers at the coffee stand and its warm light turns on.
3. Read and accept the lens at the same stand. Turn into the garden, use or bypass the first lift, cross the raised beds and stepping stones (or use the side route), then reach the projection booth via the second lift or ramp. The projector appears after delivery.
4. Read and accept the reel at the booth. Navigate traffic, ride or bypass a horizontal shuttle, ascend to the upper walk and negotiate planters. Deliver at the clocktower console. The screening lights and landscape screen reveal; a cinema ticket shows three stamps and the run statistics.

## Feedback and presentation

Cream paper panels, ink text, orange route markings and teal clock tags establish distinct roles. The HUD identifies the current parcel, destination, distance/direction, clock and Q charge. Failed actions have explicit messages and recovery. Each completed delivery changes the world, plays a stamp cue and preserves progress. Warm sunset, stationary soft clouds and distant hills support orientation. A small original musical loop and original interaction sounds can be muted. English is the startup language; six optional languages are selected from one menu.

## Comfort and accessibility

Adjust field of view and sensitivity; optionally lock camera direction; use C to aim toward the route. No camera shake, head bob, motion blur or automatic speed zoom. Reading never consumes the order budget. Pausing suspends play, retries preserve earlier work, and route alternatives reduce dependence on precise platform timing. Keyboard menu navigation is supported. Comfort, readability and full mouse interaction still require player verification.

## Iteration and evidence

The user requested a stronger opening, less generic typography, a prettier sky, enjoyable sounds and a comfortable 3D view. Technical checks also found tutorial bypasses, invalid-height interactions, ramp lips, misleading side-route navigation, default-grid terrain and an unbuilt-lighting warning. Implementations and observed regressions are recorded in ITERATIONS.md and QA-Evidence.json. These are development checks; genuine first-time player feedback and measured 5–10 minute pacing remain required.
