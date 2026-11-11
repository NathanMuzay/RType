# Release process

A release is a state of `main` that is delivered: a defense, a demo, a submission.

## Checklist

1. Merge `dev` into `main` through a pull request, after the team agrees it is stable.
2. Clean build from scratch on Linux and Windows:

    ```sh
    rm -rf build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel "$(nproc)"
    ```

3. Run the [testing checklist](../guides/engine/testing.md#before-every-merge): online R-Type with two clients, LAN
   play, Skyhop, lobby, gamepad.
4. Check the documentation builds: `mkdocs build --strict`.
5. Check the repository contains no build output, no binary and no unlicensed asset.
6. Tag the commit:

    ```sh
    git checkout main && git pull
    git tag -a defense-2 -m "Second defense"
    git push origin defense-2
    ```

## Versions

| What | Where | When to change |
|---|---|---|
| Project version | `project(RType VERSION ...)` in `CMakeLists.txt`, `vcpkg.json` | At each release |
| Protocol version | `Protocol::VERSION` | At each wire format change, see the [RFC](../reference/protocol.md) |

## Delivering binaries

Executables need the folders copied next to them by the build:

```text
bin/
├── r-type_server(.exe)
├── r-type_client(.exe)
├── skyhop(.exe)
├── data/        game files
├── scripts/     Python scripts
├── assets/      images, sounds, fonts
└── python/      Python standard library
```

On Windows, also keep the `.dll` files produced in the same folder. Zip the whole `bin` (or
`out\windows\bin\Release`) folder.
