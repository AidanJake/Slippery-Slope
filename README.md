# L10 — Animations and Parameters (Solution)

**Video Game Development (804237 DESVJ) · CITM UPC**

The completed version of `L10_Animations_Parameters`. Builds on
`L09_Map_Collision_Solution`. All the TODOs are filled in — this is what your
engine should look like at the end of part 1.

## The idea

Three things land in this lecture: the player has an animated spritesheet
instead of a static frame, the camera follows the player instead of sitting
still, and the player's tuning (texture, frame size, speed, sound) comes from
`config.xml` instead of being hardcoded — the same pattern
`Module::LoadParameters` already taught for engine modules, now extended to
entities.

None of this changes `Player`'s L08 shape: `Update()` still does the logic and
`Draw()` still does the rendering. The animation clock and the camera follow
go in `Update()`; the frame lookup goes in `Draw()`.

## What each TODO does

The numbering starts at 4 on purpose: it matches the slides.

| Marker | File | Answer |
|---|---|---|
| `L10: TODO 4` | `src/Entity.h` | `Entity::SetParameters(pugi::xml_node)` + a `parameters` member — the `Module::LoadParameters` pattern, extended to entities |
| `L10: TODO 4` | `src/Scene.cpp` | `Scene::Awake()` calls `player->SetParameters(entityManager->configParameters.child("player"))` right after `CreateEntity()` |
| `L10: TODO 4` | `config.xml` | The `<player>` section: texture path, animations TSX path, frame width/height (plus speed and the pickup sound) |
| `L10: TODO 5` | `src/Player.cpp` `Start()` | Texture, frame size and sound are read from `parameters`; `anims.LoadFromTSX(...)` with the `idle`/`move`/`jump` aliases, then `SetCurrent("idle")` |
| `L10: TODO 5` | `src/Player.h` | An `AnimationSet anims` member |
| `L10: TODO 6` | `src/Player.cpp` `Update()` | `anims.Update(dt)` — logic, so it lives in `Update()` |
| `L10: TODO 6` | `src/Player.cpp` `Draw()` | `anims.GetCurrentFrame()` + `DrawTexture(..., &currentFrame)` — rendering, so it lives in `Draw()` |
| `L10: TODO 7` | `src/Player.cpp` `Update()` | `anims.SetCurrent(...)` defaults to `"idle"` and switches to `"move"` while A/D is held |
| `L10: TODO 8` | `src/Player.cpp` `Update()` | Horizontal-only camera follow at the quarter mark, clamped to the map's edges |
| `L10: TODO 8` | `src/Map.h`/`.cpp` | `Map::GetMapSizeInPixels()` — the map's size in pixels, used by the clamp |

In the handout, the `Entity::SetParameters()` code (TODO 4) and
`Map::GetMapSizeInPixels()` (TODO 8) are already written and only marked, so
students can focus on the player; they are listed here for completeness.

This branch also ships `Assets/Textures/player2_spritesheet.tsx`, which
students build in Tiled during class, and the matching `.vcxproj` entries.

## Why the animation split matters

```cpp
bool Player::Update(float dt) {
    ...
    anims.Update(dt);     // logic: advance the animation clock
    return true;
}

bool Player::Draw() {
    SDL_Rect currentFrame = anims.GetCurrentFrame();
    Engine::GetInstance().render->DrawTexture(texture, ..., &currentFrame);  // rendering
    return true;
}
```

This is the exact `Update()`/`Draw()` boundary L04 drew for entities in
general — logic in `Update()`, rendering in `Draw()` — applied to the one new
piece of per-frame state this lecture adds.

## Why the camera follow writes `render->camera.x` directly

The camera is an `SDL_FRect` since L01, so `camera.x` is a `float` and `Render`
rounds the final screen position at draw time. Writing a float target straight
into it loses nothing, so the follow needs no helper and no shadow position.

```cpp
Engine::GetInstance().render->camera.x = camW / 4.0f - position.getX();
```

This keeps the player a quarter of the way from the screen's left edge, not
centred: a deliberate side-scroller choice that shows more of the level ahead
than behind. The follow is **horizontal only** — there is no vertical term, so
jumping does not drag the view up and down. The clamp (`limitLeft`/`limitRight`,
using `Map::GetMapSizeInPixels()`) stops the follow at the map's edges instead
of showing empty space past them.

## Why player tuning moves to `config.xml` but the spawn position doesn't

`Module::LoadParameters(pugi::xml_node)` has existed since L05, but `Player` is
an `Entity`, not a `Module` — nothing handed it a slice of `config.xml`.
`Entity::SetParameters()` is the parallel. The `<player>` node lives inside
`<entitymanager>`, so `Scene::Awake()` fetches it from
`EntityManager::configParameters` and hands it to the player by hand.

This **works because of the L04 pending queue**: `CreateEntity()` returns a
constructed-but-not-yet-`Awake`'d entity, sitting in `EntityManager`'s `pending`
list until the next `InitialisePending()` call. That is the window
`SetParameters()` needs — call it any later and `Player::Start()` would already
have read an empty `parameters` node.

It also **depends on the module order.** `entityManager` is registered before
`scene` in the `Engine` constructor, so its parameters are loaded before
`Scene::Awake()` runs. The side effect is that registration order is also
`Update()` and draw order: entities now update before the scene does.

**Initial position stays in code.** A spawn point is level data, and L15 loads
it from a Tiled object layer — putting it in `config.xml` now and moving it to
the map later would teach the wrong home for it twice. The spawn is
`(300, 300)`, clear of the left wall that L09's collision layer added.

**Animation clip names stay in code too.** `{0, "idle"}, {11, "move"}, {22, "jump"}`
are tile ids from `player2_spritesheet.tsx`'s own layout — a fact about the
asset, not a tunable.

## Other changes in this lecture

- **`Animation::Update(float dt)` takes seconds.** Frame durations and the
  accumulator are `float` seconds, converted once from the TSX's millisecond
  `duration` attribute at load time. `dt` has been in seconds since L02.
- **Physics debug draw starts off.** F1 toggles the Box2D wireframes.
- **`MapTemplate.tmx`** has colliders on all the platforms, and the
  `Collisions` layer is no longer drawn.
- **The arrow-key camera from L03** is still in `Scene::Update()`. It runs after
  the follow now, so holding Left/Right shifts the view a few pixels instead of
  scrolling it; Up/Down still move the camera vertically.

## Build

Open `PlatformGame.sln`, select **x64**, build and run. **A/D** move and **F1**
toggles the Box2D debug wireframes. The player should visibly walk instead of
sliding as a static frame, and the camera should start scrolling once you walk
far enough from the quarter-mark position.

## Read the code

Start at `Player::Start()` for how `parameters` becomes a texture, a frame size
and an `AnimationSet`. Then `Animation::Update()`/`AnimationSet::LoadFromTSX()`
in `src/Animation.cpp` for the clip machinery itself (given, not a TODO — you
call it, like `Physics` at L08). Then the camera follow at the end of
`Player::Update()`.

## Homework

- Compare this branch against `L10_Animations_Parameters` — the diff should be
  the TODO bodies plus the TSX.
- The `"jump"` clip is registered but the player cannot jump yet. Add a jump,
  and switch to the clip when the jump starts and back on landing.
- If the player TSX is missing, `AnimationSet::LoadFromTSX()` logs and returns
  `false`, silently leaving the player idle-only. Given L05's "hard-fail on bad
  config" precedent, should a missing player TSX fail the same way?

## Reference

- Tiled: tile animations — <https://doc.mapeditor.org/en/stable/manual/editing-tilesets/#tile-animation-editor>
- A1 spec: "Parameters loading — read configurations and entity parameters
  from external files: Player parameters (initial position, speed, tiles)"
