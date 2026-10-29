# Last Minute Courier

Design draft v0.1, October 29, 2026. Planned features below have not yet been implemented or playtested. Final structure must be checked against lecture deck 3.

## Concept

A cheerful third-person 3D delivery game in which a campus courier borrows three seconds of slow motion to bring a movie night together. Run through moving cart lanes, jump onto service lifts, and choose when to spend your next charge. Each delivery lights up another part of the same evening scene.

## Audience and player role

Designed for a single player who understands basic keyboard movement but may be unfamiliar with action games. The opening practice yard introduces movement, camera control, jumping and selective slow motion without a deadline. Wide paths, short recovery distances and order-level retries support new players. More confident players can select shortcuts and improve their delivery times.

## Theme and background

The project theme is Time. The laboratory's Borrower slows campus transport devices marked with blue time tags. The courier and delivery clock continue normally. This limitation is explained before play and gives players a reason to think about timing rather than treating slow motion as a way to stop the deadline.

The campus film screening is about to begin. A coffee stand needs its insulated box, the projection booth needs its lens, and the clocktower console needs the opening reel. The courier's three deliveries progressively prepare the square. At the end, three delivery stamps form a ticket: the courier has a seat at the screening too.

## Core experience

Collect an order, read its destination and deadline, traverse a short route, and deliver it. Collect the next order at the same destination. Slow motion changes both obstacle avoidance and the timing window for boarding moving platforms. The third route combines these uses with a choice of path.

## Distinguishing details

- One ability with multiple applications and an explicit recharge interval.
- A connected level with three purposeful deliveries and visible changes to the shared destination.
- A warm ending that rewards completing the work, without requiring a high score.

## Scope and modes

One single-player level, intended to take 5–10 minutes on a first completed run. The experience includes an opening story sequence, untimed practice, timed delivery, pause/help, order results, order retry and final completion. These are the planned interaction states; the course's precise use of Game Modes remains to be checked against the lecture.

## Learning goals

Players learn to read moving obstacles, use slow motion to enlarge safe movement windows, transfer that understanding to moving platforms, and plan their route around a limited charge. The final route should demonstrate independent use rather than simply repeating the tutorial.

## Implementation and delivery

Unreal Engine, 3D, keyboard and mouse. Target releases: Windows, macOS and Linux, with architectures to be confirmed against grading hardware. macOS is the initial development machine, not the only required release. A playable build and build sizes have not yet been validated.

## Validation plan

Use real first-time playtests to evaluate instruction clarity, camera comfort, route readability, ability transfer, failure recovery and completion time. Document feedback-driven revisions with corresponding Git commits. Do not describe planned changes as completed iterations.
