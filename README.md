# Auto Decorator

Cross-platform Geode mod for Geometry Dash 2.2081. It adds a **Decorate**
button to the level editor and places deterministic decorative details around
the currently selected objects.

## Usage

1. Select the blocks or structures you want to decorate.
2. Press **Decorate** in the upper-right corner.
3. Use Geometry Dash's Undo button if you do not like the result.
4. Open Geode → Auto Decorator → Settings to change density, spread, scale,
   object ID, seed, and the safety limit.

The default object ID is `196` (glow). Changing the object ID makes the mod a
general scatter tool; use non-gameplay decoration objects to avoid altering
the level's gameplay.

## Building

Install the current stable Geode CLI and SDK:

```sh
geode sdk install
geode sdk install-binaries
```

Build for the current desktop platform:

```sh
geode build
```

Android builds:

```sh
geode sdk install-binaries -p android64
geode build -p android64

geode sdk install-binaries -p android32
geode build -p android32
```

The GitHub Actions workflow builds Windows, macOS, Android 32-bit, and Android
64-bit packages and combines them into one `.geode` file.

## Notes

- A single click creates at most the configured safety limit (500 by default).
- Generation is deterministic. Change **Seed** for another layout.
- The mod never runs automatically and never edits unselected structures.
