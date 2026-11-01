# Pocket Tumbler

A virtual pet–style idle game for **Pebble**, but for rock polishing. Start with a
jagged gray stone on the e-paper display. Walk in the real world — the watch
pedometer tumbles the rough rock. Hit your weekly step goal and it gradually
smooths out, eventually revealing a brilliant gemstone (full 64-color sparkle on
color Pebbles).

## How it works

1. Open the app — your current rock sits in a little tumbler drum.
2. **Walk.** Weekly steps from Pebble Health polish the stone.
3. **Shake / move the watch** — the IMU spins the rock in the barrel (optional
   small daily polish bonus).
4. Hit the weekly step goal for a **10% polish bonus**.
5. When the gem is ready, press **Select** to collect it and start a new rough rock.

| Button | Action |
| --- | --- |
| Select | Claim finished gem → new rock |
| Up | Refresh step totals |
| Down | Nudge tumble animation |

Stages: Rough → Angular → Softened → Smooth → Polished → Gleaming → **Gem**
(Ruby, Sapphire, Emerald, Amethyst, Topaz, Citrine, Aquamarine, or Garnet).

## Settings

In the Pebble phone app: **Pocket Tumbler → Settings** (Clay, works offline):

- **Weekly step goal** — 20k / 35k / 50k / 70k
- **Weeks to gem** — how many goal-weeks of walking to fully polish (3 / 5 / 8)
- **Shake bonus** — IMU tumble can add a little polish (capped per day)
- **Vibrate** — buzz on weekly goal and gem reveal

## Build and install

The Pebble SDK runs on Linux and macOS only. On **Windows**, `build.ps1` runs it in
a container (install [Podman Desktop](https://podman-desktop.io) or Docker Desktop first):

```powershell
./build.ps1
```

Output: `build/PocketTumbler.pbw`.

On **macOS/Linux**, [install the SDK](https://developer.repebble.com/sdk) and run
`pebble build` from this folder (or a path without spaces).

### Put it on your watch

- Send `build/PocketTumbler.pbw` to your phone and open it with the Pebble app, or
- Turn on the developer connection and run:

  ```powershell
  ./build.ps1 -Phone 192.168.1.23
  ```

- For CloudPebble: `./build.ps1 -Zip` → `dist/PocketTumbler-cloudpebble.zip`

## Notes

- Requires **Pebble Health** enabled for pedometer polishing. Without it, you can
  still shake for a little bonus polish and see the tumbler animation.
- Color watches get the 64-color gem reveal; B&W Pebbles use high-contrast
  silhouettes and dithered stages.
- Progress is stored in watch persistent storage.

## Layout

```
src/c/
  main.c       lifecycle, HealthService, IMU, buttons
  game.*       polish XP, weekly steps, gem claim, persistence
  tumble.*     accelerometer energy → rock spin
  rock.*       procedural jagged→gem silhouettes
  draw.*       barrel UI + HUD
  settings.*   goals / vibe / shake bonus
  comm.*       AppMessage from Clay
src/pkjs/      Clay settings (config.js + index.js)
resources/images/menu_icon.png
```
