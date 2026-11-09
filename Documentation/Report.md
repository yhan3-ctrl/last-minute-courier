# Last Minute Courier

Project 3 | Design and Iteration Report

**Yu Han** | University of Mississippi | November 8, 2026

## 1. High Concept

A campus movie night is missing its coffee, projector lens and final reel. As the last courier on shift, the player borrows seconds from blue-tagged machinery, combines timing with parkour movement, and delivers the three parcels to earn a seat at the screening.

This is a single-player, third-person 3D action and timing game made in Unreal Engine 5.8.4. The player observes a moving obstacle, chooses a route, then sprints, jumps, climbs or slides through a safe window. The Borrower slows marked machinery without slowing the courier or the delivery deadline. Time is therefore a decision the player makes, not simply the story's theme.

The intended audience understands keyboard and mouse controls but may be new to 3D action games. Three opening cards and an untimed practice yard introduce the story, the device and the controls before any delivery clock starts. Wide paths, nearby checkpoints and longer alternatives support cautious play. Optional roofs, stepping stones and a springboard offer faster, more demanding lines.

The complete level connects the parcel desk, coffee plaza, garden, projection booth and clocktower cinema. Its design target is a first completed session of 5-10 minutes, without filler. Each delivery contributes to the same evening: coffee supplies the audience, the lens enables projection, and the reel starts the film. Three delivery stamps become one admission ticket.

### Presentation and player experience

Cream buildings, terracotta details and a warm sunset make a small, readable campus. Blue clock tags identify machinery affected by Q; orange paint identifies the recommended route; attracting gold hourglasses and spark trails identify rewards. Stationary clouds and distant hills provide orientation.

Menus resemble courier slips, with a coloured header, paper body, perforation and key-cap controls. Lilita One provides display lettering; Nunito provides interface text; Noto Sans supports Chinese, Japanese and Korean. English is the default. A language menu also offers Chinese, Japanese, Korean, Spanish, French and German.

An original 48-second musical base gains synchronized strings after coffee and bells after the lens. Pickups, borrowed time, impacts and delivery stamps have distinct audio cues. A sound switch controls music and effects. The camera has no head bob, shake or motion blur, with adjustable sensitivity, field of view and optional direction lock. Sprinting and sliding gently widen the view; V recenters it.

The accompanying High Concept and Game Script provide the independent design documents. This report connects the design to the graduate analysis, playtesting and iteration history.

## 2. Game Script: teaching and progression

The opening establishes who the player is, why the parcels matter, and how to use the Borrower. The practice yard then teaches movement without a delivery deadline. Only accepting an order starts its clock; reading a brief is always untimed.

| Ability | Introduction | Later application |
|---|---|---|
| Move, look and sprint | Key-cap card, safe yard and sprint prompt. | Choose a longer safe path or a faster line; sprint to a departing platform or into the springboard. |
| Jump and climb | Physically jump the practice step; the first crate wall explains jumping into a ledge to climb it. | Cross stepping stones, clear a low turnstile arm, climb crate stairs to the market roof. |
| Slide | C or Ctrl slides under a practice beam spanning the yard. | Pass a closing shutter or the high turnstile arm; combine slide timing with Q. |
| Borrow time | Blue tags and the Q meter explain a temporary slowdown; practice offers a cart-lane example. A safe crossing also works without Q. | Slow a lift or shuttle to board, then release Q for travel; slow a shutter or turnstile to combine movement with a readable gap. |
| Interact | Reaching the desk opens its brief; E or Enter accepts. | Deliver at the marked destination, then read and accept the next parcel. |

Teaching progresses from isolated actions to combinations. The first crate climb is introduced on the coffee route before later roof use. Some platform and climb challenges have longer alternatives; the three turnstile gates are compulsory.

### Complete player sequence

**Opening and practice.** Read three cards: the film-night story, borrowed time, and controls. Practice sprinting, jumping and sliding; observe the cart and cross after it passes, optionally extending the gap with Q. The route leads to the parcel desk. The yard exit stays closed until the first parcel is accepted.

**Coffee.** Accept the coffee, cross the traffic or take the arcade, and meet the first crate wall and rolling shutter. Clear the turnstile gate and deliver at the coffee stand. Coffee warmth determines stars. Cups, steam, lights and the strings layer make the first contribution visible and audible.

**Lens.** Accept the lens at the coffee stand. Navigate the garden's lift or ramp, raised beds and stepping stones or side walk; combine Q with the turnstile's jump/slide pattern. Reach the projection booth by lift or ramp and pass its shutter. Hits and falls damage the case. Delivery prepares the projector, brings the audience and adds bells.

**Reel.** Accept the reel at the booth. Use traffic gaps or the market roof, pass the market shutter, ride or bypass the shuttle, clear the final turnstile and reach the upper walk by lift or springboard. Follow the central cinema aisle to the marked console. Delivery reveals the screening and a ticket showing three stamps, stars and the run's rank.

## 3. Game Script: rules and all modes

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

### Gameplay modes and shell screens

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

## 4. Graduate analysis: seven mechanic categories

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

### How the Time theme changes play

The Borrower grants a local opportunity instead of pausing the whole game. Slowing traffic can make a gap easier to cross, but slowing a lift for too long also delays its arrival. Releasing Q is therefore a meaningful action. A fixed per-order allowance prevents holding the power continuously, while hourglasses and spark trails create a choice between safety now and more options later.

The parcels give the deadline different meanings. Coffee loses warmth, the lens demands care, and the reel must reach the screening. These concerns lead to different star rules while retaining one consistent movement and time-control system.

## 5. Balance lens: Reward

**1. What rewards exist, and what other forms are possible?** Deliveries give stamps, confetti, a sound cue and a persistent contribution to film night. Hourglasses grant one Q use; 25 sparks grant another; close calls grant sparks. Parcel-specific stars combine into the final ticket's rank. Cosmetic unlocks are possible, but are unnecessary for this short campaign.

**2. Are rewards exciting or boring, and why?** The intended appeal is both immediate feedback and a larger shared goal: a cup-filled stand, a working projector, an audience and finally the film. Player feedback described the earlier experience as monotonous and too easy. More expressive traversal, spark trails and parcel-specific scoring respond to that concern rather than simply increasing collectible counts.

**3. Do players understand the reward?** The earlier blue pickup looked dangerous, and players avoided it. The revision makes it an attracting gold hourglass, distinct from blue machinery, with a pickup cue and an explicit extra-use message. Destination stamps and visible world changes connect a delivery to its outcome; three stamps become the admission ticket.

**4. Is reward timing too regular, and should it vary?** Three major delivery rewards are predictable because the story has three concrete needs. Smaller sparks, hourglasses and close-call bonuses vary with movement and route risk between them. Random schedules would weaken the link between a successful action and its result.

**5. How are the rewards connected?** Coffee supplies the audience, the lens enables projection and the reel provides the film. The musical arrangement accumulates with those contributions. The ticket reuses the same delivery stamps, so the ending belongs to the player's work rather than appearing as an unrelated prize.

**6. Does the reward build too quickly or slowly?** The intended progression moves from a small local response to a populated terrace and a complete screening. Optional stars and riskier spark lines provide a reason to improve a finished run. Reward pacing is adjusted alongside movement speed and route difficulty; compulsory waiting or repeated text is not used to lengthen play.

### Implementation consequences

Rewards support the same action vocabulary rather than introducing a separate economy. A harder line can replenish the resource used to attempt it. Stars recognize the customer's concern, while delivered stamps are preserved even after a later retry. The player can finish the story with a modest score and then replay for a cleaner ticket.

## 6. Balance lens: Punishment

**1. What punishments exist?** Moving-obstacle contact causes knock-back and a brief stagger; a fall returns to a checkpoint. During the lens order, hits and falls damage the case and the third fails that attempt. A deadline failure requires retrying only the unfinished order. There is no finite life pool or loss of earlier delivery stamps.

**2. Why punish, and what should it achieve?** A setback gives traffic timing and safe landings a consequence. A local retry lets the player apply the timing just learned without repeating completed deliveries. Its purpose is attention and improvement rather than humiliation or artificial length.

**3. Does the punishment feel fair, and why?** Earlier practice feedback identified an unfair rule in earlier practice: crossing safely after the cart passed still caused a reset because Q was required. That reset was removed. Later, overly fast turnstiles and unclear key response led to slower rotations, gentler collision margins, the C slide key and lit key caps. The design now ties setbacks to visible contact or falling, with a written reason.

**4. Could a reward produce the same or a better effect?** Clean crossings already preserve deadline time, lens condition and stars. Checkpoints and persistent stamps reward progress. The game therefore avoids additional arbitrary time penalties or subtracting earlier stamps; it keeps enough consequence to make machinery matter.

**5. Are strong penalties matched by strong rewards?** The largest ordinary setback is replaying one order. Its reward is a permanent contribution to the screening, culminating in the film and ticket. Parcel budgets and checkpoints limit repeated work, while the stronger lens condition rule gives careful play a distinct purpose. A full campaign reset would be disproportionate for this audience.

### Recovery is distinct from retry

A fall is a local recovery while the active clock continues. A timeout or broken lens ends the attempt and offers a fresh brief. A requested restart first shows a confirmation. These states have different consequences and are described separately in the Game Script and player-facing messages.

## 7. Balance lens: Simplicity and complexity

**1. What innate complexity exists?** Players must understand the blue-tag rule, Q duration and recharge, the per-order use count, reading versus accepting, parcel order and recovery versus retry. Cards introduce the controls; practice demonstrates movement; briefs show the parcel's budget and allowance; contextual hints explain the current obstacle.

**2. Can innate complexity become emergent complexity?** The same Q rule affects carts, lifts, the shuttle, shutters and turnstiles. It combines with existing movement instead of adding a separate control system for each device. A later route can therefore ask a new question - when to release Q after boarding, or how to fit a slide under a slowing door - without adding another power.

**3. What emergent interactions arise?** Slowing a cart extends a crossing window but still consumes deadline time. Slowing a lift helps boarding but delays travel if held too long. A longer ramp reduces precision demands at the cost of time. A harder spark line can replenish the use spent reaching it. Observation and route choice produce these tradeoffs from a small rule set.

**4. Is anything too simple?** Earlier feedback said the game was too easy and monotonous. Parkour combinations and compulsory turnstile gates add active decisions. Optional ramps and bypasses remain to support novices. Removing the forced-Q practice reset avoids mistaking an arbitrary requirement for useful complexity; slowing the turnstiles avoids confusing difficulty with unreadable motion.

### Interface clarity

The play screen uses small corner plates, one contextual instruction and an interaction prompt. Extended explanations live in untimed cards, briefs and help. The independent High Concept and Game Script organize the premise and the implemented rules into their respective documents.

This division follows the player's immediate question: where am I going, how much time remains, what can I do here? Route paint, a next-waypoint ring and moving-platform chevrons provide spatial answers. Key caps illuminate during the action, so an apparently ignored input has visible feedback. The interface preserves information without making every rule a permanent paragraph.

## 8. Player feedback and meaningful iteration

Feedback from four players on v0.9/v0.10 informed the latest revisions. The table also includes earlier practice, route and collection observations. Each revision addresses a specific problem in understanding, movement or challenge.

| Feedback or observation | Change and design reason |
|---|---|
| A safe crossing without Q caused a reset; the route was faint. | v0.5.3 removed the forced-Q reset and strengthened the orange line, large arrows and next waypoint. Natural timing is a valid solution. |
| Players followed the route without a parcel; E at the destination seemed ineffective. | v0.6 opens the brief on approach, blocks the first exit until acceptance, brings the counter within reach and explains interaction targets. |
| Traffic and collision behavior felt illogical; route paint disagreed with the lift marker. | v0.6-v0.8 keep machines moving, use one-way depot traffic and knock-back, paint one recommended line and place chevrons on moving platforms. |
| Rewards looked unsafe; the game felt monotonous and too easy. | v0.7-v0.9 add attracting gold hourglasses, sparks, stars, parcel-specific concerns and sprint/slide/climb combinations. Blue remains the device cue. |
| Turnstiles were too fast, could be avoided, and keys seemed unresponsive. | v0.10 slows periods to 10/8.5/7.5 s, joins walls to hedges, makes C the main slide key, buffers a slide pressed in air and illuminates key caps. |
| Review found a shutter obscuring the camera at the second destination. | v0.10.1 adds camera blocking to the visible door while retaining the independent movement blocker; comfort text and application version metadata are synchronized. |

### Revision history and verification

The repository retains fifteen iteration tags from v0.1 to v0.10.1. ITERATIONS.md records the problem, the revision and the check for each stage; README provides a shorter index. The v0.10.1 revision adds the camera and documentation corrections described here.

Verification combines engine regression for boundaries, parkour, platform riding and three-order progression; localization coverage; and a packaged journey reviewed through screenshots. Feedback guides the design, while these checks verify that the corresponding behavior is implemented. The feedback record is retained in PlaytestFeedback-Nov08.md.

## 9. Rubric alignment

| Rubric focus | Evidence in this submission |
|---|---|
| A playable release for the instructor | A self-contained Mac application and release package; platform and repository information in section 10. |
| A 3D Unreal game | Unreal Engine 5.8.4 project, third-person courier, connected 3D campus and moving devices. |
| Teach abilities, then apply them in new ways | Section 2 maps introduction to later use: jump/climb, slide and borrowed time combine at shutters, turnstiles, platforms and the roof. |
| 5-10 minutes without filler | First-session design target in section 1; three purposeful deliveries, optional route choices, untimed reading and local retries. |
| An enjoyable game | Linked world rewards, parkour, parcel-specific stars and music; section 8 explains changes responding to boredom, reward confusion and difficulty. |
| Complete High Concept and Game Script in the correct documents | Separate High Concept and Game Script PDFs accompany this report. The latter includes the full graduate analysis. |
| Accurate account of what the game teaches | Section 2 and the Game Script's teaching/transfer table identify actual practice actions and later combinations. |
| Detailed, understandable rules | Section 3 states clocks, inputs, Q, pickups, collisions, stars and progress; the Game Script expands edge cases. |
| All experienced modes defined | Section 3 distinguishes practice and delivery from cards, briefs, pause/help, confirmation, results, failure and completion. |
| Concept, player and background | Section 1 identifies the courier role, campus movie night, audience, interaction model and intended experience. |
| Iteration descriptions match repository history | Section 8, ITERATIONS.md and the fifteen tags trace development from v0.1 through v0.10.1. |

### Graduate requirements

Section 4 classifies mechanics as Space, Time, Objects/attributes/states, Actions, Rules, Skill and Chance. Sections 5-7 answer every question for Reward (six), Punishment (five), and Simplicity/Complexity (four), exceeding the minimum of two lenses. Answers connect the lenses to implemented choices and recorded feedback rather than listing definitions alone.

The separate Game Script and Graduate Analysis retain the more detailed descriptions. The source design documents are in the repository's Documentation directory, and their PDFs are in output/pdf.

## 10. Repository, documents and running the game

**Repository:** https://github.com/yhan3-ctrl/last-minute-courier

**Published Mac release:** https://github.com/yhan3-ctrl/last-minute-courier/releases/tag/v0.10.1

The repository contains the Unreal project, source, design data, build scripts, asset credits, iteration history and documentation. Tags v0.1-v0.10.1 preserve the development stages. ITERATIONS.md explains the changes in each version.

### Files to submit together

| Document | Role |
|---|---|
| High Concept | Independent concept document: premise, role, primary gameplay, player, world, perspective and scope. |
| Game Script and Graduate Analysis | Independent game script: teaching, rules, complete flow, all modes, seven mechanics and three balance lenses. |
| Project Report | This concise rubric-aligned report, feedback-to-revision account and submission guide. |

Asset and font attribution is retained in ASSET_CREDITS.md and the included licenses.

### Running the Mac release

Open the DMG, copy LastMinuteCourier.app to a writable local folder, and open the application. It is a standalone Apple-silicon application; Unreal Editor is not required to play. Keep the application bundle intact. Read the three opening cards, optionally choose a language, then complete practice and accept the first parcel.

Core controls are WASD and mouse; Shift sprint; Space jump/climb; C or Ctrl slide; Q borrowed time; E interact; V recenter; H help; Esc pause. The pause menu includes camera, sound and language settings. Reading and settings are untimed.

### Platform handoff

Target platforms are Windows and Linux on Intel/AMD hardware, and macOS on Apple silicon. BuildOtherPlatforms.md and the platform scripts document the build workflow. Box is the course distribution location for the platform-specific packages; GitHub contains source and iteration history.

### Design references

Project 3 assignment brief, three pages; lecture 3, Fundamentals of Game Design, for concept/script structure and gameplay modes; lecture 4, The Art of Game Design, for the seven mechanic categories and the Reward, Punishment and Simplicity/Complexity lenses. Implementation details and revision sources are maintained in HighConcept.md, GameScript.md, GraduateAnalysis.md and ITERATIONS.md.
