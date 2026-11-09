# Player feedback and revisions - November 8, 2026

## Evidence source

The owner reports four people played v0.9/v0.10, with similar concerns, and confirms that changes were made immediately in response to their feedback. The following is an aggregate retrospective account reconstructed from the owner's messages and the existing iteration history. It is not four separate interview transcripts. Individual durations, completion counts, dates and verbatim quotations were not recorded. These four reported players are not added to older session counts, which may overlap.

## Feedback-to-change trace

| Reported concern | Revision and rationale | Verification and remaining observation |
|---|---|---|
| Earlier practice rejected the intuitive strategy of crossing after a cart passed. The route was hard to see. | v0.5.3 accepted safe crossings with or without Q; strengthened the continuous orange route and next waypoint. | Boundary regression accepts natural timing. Independent learning and total duration are separate questions. |
| Players could walk to the destination without collecting a parcel; E appeared unresponsive. | v0.6 automatically opens the desk brief, blocks the first exit until acceptance, brings the counter within reach and explains E when there is no target. | Desk, gate, interaction and brief/acceptance regression checks. |
| Stationary traffic, abrupt collision resets and a route that disagreed with the lift marker felt illogical. | v0.6-v0.8 kept machinery moving, added one-way depot traffic and knock-back, painted only the recommended route, and placed chevrons on moving platforms. | Traffic, route-floor, marker and platform checks; complete scripted journey. |
| Rewards looked unsafe to touch; the experience felt monotonous and too easy. | v0.7-v0.9 added parcel-specific stars and world responses, attracting gold hourglasses, spark trails, sprint/slide/climb and combinations at shutters and turnstiles. | Reward counting, lens damage, movement and obstacle tests. This establishes implementation, not a positive enjoyment rating. |
| v0.9 turnstiles were too fast, could be bypassed, and key response was unclear. | v0.10 slowed rotations to 10/8.5/7.5 s; joined walls to hedges; made C the main slide key, retained Ctrl, buffered an airborne slide and lit the active key caps. | Four engine suites and the v0.10 normal-input scripted journey passed. Reported group concerns motivate the changes; post-fix subjective results are unrecorded. |

Earlier rows retain the previously documented partial owner sessions; they are not attributed independently to all four later players. Source commits and tags are listed in ITERATIONS.md.

## Technical review on November 8

A new ordinary-input packaged run of v0.10 completed all three deliveries in 143.0 seconds (orders 8.1/42.4/56.3 seconds), with no recovery, timeout, retry or stuck event and 9/9 stars. The route-following driver knows all crossings and reads cards for fixed short intervals. Its duration is an expert technical lower bound, not a first-time player's duration. A separate controlled UI tour deliberately forces a timeout and a broken lens to inspect failure screens; its timings and failures must not be treated as normal play.

Screenshot review found that the second destination's shutter could cover the chase camera even when it did not block the courier. v0.10.1 makes the visible door block the camera only, keeping the separate movement blocker and order rules unchanged. A regression trace checks this distinction. The comfort description now acknowledges the existing gentle sprint/slide field-of-view change in all seven languages. Version metadata is corrected from the stale 0.5.3 value to 0.10.1.

## Acceptance boundaries

Four reported players do not establish four completed runs or a 5-10-minute first-run duration. No individual times, completion counts or quotations have been inferred from source changes. Final fun, independent learning, comfort, listening and native-language quality remain player observations. The submission report concentrates on design, actual aggregate feedback and revisions; this technical evidence stays in the audit record.
